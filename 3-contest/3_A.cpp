#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "wavreader.h"
#include <algorithm>
#include <cassert>

const int base = 10;
const int num_base = 1;
const double pi = 3.141592653589793;

struct Complex{
    double Re, Im;

    Complex(double x=0, double y=0) : Re(x), Im(y) {}
    //Complex(int x=0) : Re(x), Im(0) {}
    //Complex(size_t x=0) : Re(x), Im(0) {}

    Complex& operator *= (const Complex& a){
        double tmp_re = Re, tmp_im = Im;
        Re = tmp_re * a.Re - tmp_im * a.Im;
        Im = tmp_im * a.Re + a.Im * tmp_re;
        return *this;
    }
    Complex& operator += (const Complex& a){
        Re = Re + a.Re;
        Im = Im + a.Im;
        return *this;
    }
    Complex& operator -= (const Complex& a){
        Re = Re - a.Re;
        Im = Im - a.Im;
        return *this;
    }
    //division by int
    Complex& operator /= (const int a){
        Re /= a;
        Im /= a;
        return *this;
    }
};

Complex operator * (const Complex& a, const Complex& b) {
    Complex c = a;
    c *= b;
    return c;
}

Complex operator + (const Complex& a, const Complex& b) {
    Complex c = a;
    c += b;
    return c;
}

Complex operator - (const Complex& a, const Complex& b) {
    Complex c = a;
    c -= b;
    return c;
}

//convertion from string
std::vector<Complex> convert(const std::string &s){
    if(s.empty())
        return{};

    std::string str = s;

    if (str[0] == '-') //if negative
        str.erase(0, 1);

    size_t str_size = str.size();

    size_t size_vec = ((str_size % num_base) == 0 ? str_size / num_base : str_size / num_base + 1);//size of vector
    std::vector<Complex> digits(size_vec, 0);

    size_t last_size = str_size % num_base;

    size_t cur = 0;
    size_t count = 0;

    for(size_t i = 0; i < str_size; ++i){
        cur *= 10;
        cur += str[i] - '0';

        if (i + 1 >= last_size && (i + 1 - last_size) % num_base == 0) {
            digits[size_vec - count - 1] = cur;
            cur = 0;
            ++count;
        }
    }
    return digits;
}


void fft(std::vector<Complex> &v){
    size_t size = v.size();

    //TODO count vector of indexes
    for (int i=1, j=0; i<size; ++i) {
        int bit = size >> 1;
        for (; j>=bit; bit>>=1)
            j -= bit;
        j += bit;
        if (i < j)
            std::swap(v[i], v[j]);
    }

    for (size_t n = 2; n <= size; n <<= 1) {
        double angle = -2 * pi / n;

        Complex w(cos(angle), sin(angle));
        for (size_t i = 0; i < size; i += n){
            Complex cur_w(1, 0);

            for (size_t j = 0; j < (n >> 1); ++j) {
                Complex u_1 = v[i + j],  u_2 = v[i + j + (n >> 1)] * cur_w;
                v[i + j] = u_1 + u_2;
                v[i + j + (n >> 1)] = u_1 - u_2;

                cur_w *= w;
            }
        }
    }
}

void ifft(std::vector<Complex> &v){
    size_t size = v.size();

    //TODO count vector of indexes
    for (int i=1, j=0; i<size; ++i) {
        int bit = size >> 1;
        for (; j>=bit; bit>>=1)
            j -= bit;
        j += bit;
        if (i < j)
            std::swap(v[i], v[j]);
    }

    for (size_t n = 2; n <= size; n <<= 1) {
        double angle = 2 * pi / n;

        Complex w(cos(angle), sin(angle));
        for (size_t i = 0; i < size; i += n){
            Complex cur_w(1, 0);

            for (size_t j = 0; j < (n >> 1); ++j) {
                Complex u_1 = v[i + j],  u_2 = v[i + j + (n >> 1)] * cur_w;
                v[i + j] = u_1 + u_2;
                v[i + j + (n >> 1)] = u_1 - u_2;

                cur_w *= w;
            }
        }
    }

    for (size_t i = 0; i < size; ++i)
        v[i] /= size;
}

void normalize(std::vector<Complex>& v){
    size_t j = v.size() - 1;

    while (v[j].Re == 0 && j > 0)//избавляемся от последних нулей
        --j;
    ++j;
    v.resize(j);

    for(size_t i = 0; i < v.size(); ++i){//нормализация
        if (v[i].Re >= base){
            if (i == v.size() - 1) {
                v.push_back(v[i].Re / base);
                v[i] = int(v[i].Re + 0.5) % base;
                break;
            }
            else {
                v[i + 1] += int(v[i].Re + 0.5) / base;
                v[i] = int(v[i].Re + 0.5) % base;
            }
        }
    }
}

void long_multiply(const std::string &a, const std::string &b){
    //TODO sign!!!
    std::vector<Complex> v_a = convert(a), v_b = convert(b);

    size_t common_size = 1;
    while (common_size < std::max(v_a.size(), v_b.size()))
        common_size <<= 1;
    common_size <<= 1;

    v_a.resize(common_size, 0),  v_b.resize (common_size, 0);

    fft(v_a), fft(v_b);

    for (size_t i = 0; i < common_size; ++i)
        v_a[i] *= v_b[i];

    normalize(v_a);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//read from 256 to 10
double get_sample(unsigned char *entry, int sample_size){
    size_t res = 0, pow = 1;
    //numbers in 256 system
    //std::cout << *(entry) << " " << *(entry+1) << "\n";
    for(int i = 0; i < sample_size; ++i){
        res += ((*entry) - 0) * pow;
        ++entry;
        pow *= 256;
    }

    return res;
}

//write from 10 in 256
void write_sample(int x, int fd, int sample_size){
    std::vector<unsigned char> s(sample_size);

    for(int j = sample_size - 1; j >= 0; --j){
        s[j] = (unsigned char)(x % 256);
        x /= 256;
    }
    std::reverse(s.begin(), s.end());

    ///////TEST/////////
    //std::cout << s[0] - 0 << " " << s[1] - 0 << "\n";
    write(fd, &s[0], sample_size);
    //    for(int k = 0; k < sample_size; ++k){
//        write(fd, &s[k], 1);
//    }
}

void write_to_file(std::vector<Complex> v, char *header, int sample_size){
    char path_name[] = "result.wav";
    int fd = open(path_name, O_CREAT | O_WRONLY, S_IRWXU);
    write(fd, header, 44);

    //////////TEST///////
    for(int i = 0; i < v.size(); ++i){
        write_sample(int(v[i].Re), fd, sample_size);
    }

    close(fd);
}

void check(){
    std::cout << "---------------------" << "\n";
    int fd1 = open("speech.wav", O_RDONLY);
    int fd2 = open("result.wav", O_RDONLY);
    void *entry1 = mmap(NULL, 108524, PROT_READ, MAP_PRIVATE, fd1, 0);
    void *entry2 = mmap(NULL, 108524, PROT_READ, MAP_PRIVATE, fd2, 0);
    unsigned char* e1 = static_cast<unsigned char *>(entry1);
    unsigned char* e2 = static_cast<unsigned char *>(entry2);

    for(int  i = 0; i < 108524; ++i)
        if(*(e1 + i) != *(e2 + i)) {
            std::cout << i << ":" << *(e1 + i) - 0 << " " << *(e2 + i) - 0 << "\n";
        }

    munmap(entry1, 108524);
    munmap(entry2, 108524);
    close(fd2);
    close(fd1);
}

void read_from_file(){
    FILE *file = fopen("speech.wav", "r");
    if (!file){
        std::cout << "Failed open file"; return;
    }

    WAVHEADER header;
    fread(&header, sizeof(WAVHEADER), 1, file);
    int data_size = header.subchunk2Size;
    int sample_size = header.blockAlign;
    fclose(file);

    int fd = open("speech.wav", O_RDONLY);
    char string_header[44] = {0};
    read(fd, string_header, sizeof(string_header));

    struct stat cur_stat;
    fstat(fd, &cur_stat);
    size_t all_size = cur_stat.st_size;

    void *entry = mmap(NULL, all_size, PROT_READ, MAP_PRIVATE, fd, 0);

    std::vector<Complex> v;
    unsigned char* sample_entry = static_cast<unsigned char*>(entry);
    sample_entry += 44;

//    for(int i = 0; i < 50; ++i){
//        std::cout << *(sample_entry + i) - 0 << " " << *(sample_entry + i + 1) - 0 << "\n";
//    }

    std::cout << "--------------------------\n";

    //getting polynomial
    for(int i = 0; i < data_size / sample_size; ++i) {
        v.push_back(get_sample(sample_entry, sample_size));
        sample_entry += sample_size;
    }

    munmap(entry, all_size);
    close(fd);

    //get min pow of 2 for proc size
    size_t proc_size = 1;

    while (proc_size < v.size())
        proc_size <<= 1;

    printf("Sample size %d\n", sample_size);
    printf("Data size %d\n", data_size);
    printf("Proc size %ld\n", proc_size);

    v.resize(proc_size);

    fft(v);
    int compr_start = proc_size * 99 / 100;
    std::fill(v.begin() + compr_start, v.end(), 0);
    ifft(v);

    write_to_file(v, string_header, sample_size);
    //check();
}

int main() {
    read_from_file();
    return 0;
}
