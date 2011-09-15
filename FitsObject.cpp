#include "FitsObject.h"
#include <stdexcept>

using namespace std;

    Fits::Fits(const string &filename)
: m_status(0), m_filename(filename)
{
    fits_open_file(&this->m_fptr, this->m_filename.c_str(), READWRITE, &this->m_status);
    this->check();
}

Fits::Fits() {}

Fits::~Fits()
{
    fits_close_file(this->m_fptr, &this->m_status);
    this->check();
}

void Fits::check()
{
    if (this->m_status)
    {
        char buf[FLEN_STATUS];
        fits_get_errstatus(this->m_status, buf);

        /* Have to set the status back to 0 otherwise
         * when the destructor is called and the file is closed
         * then another exception will be thrown */
        this->m_status = 0;
        throw runtime_error(buf);
    }
}

void Fits::moveHDU(const string &hduname)
{
    fits_movnam_hdu(this->m_fptr, ANY_HDU, const_cast<char*>(hduname.c_str()), 0, &this->m_status);
    this->check();
}

void Fits::moveHDU(int hdunum)
{
    int hdutype;
    fits_movabs_hdu(this->m_fptr, hdunum, &hdutype, &this->m_status);
    this->check();
}

long Fits::nrows()
{
    // Ensure the current hdu is a (binary) table 
    int hdutype;
    fits_get_hdu_type(this->m_fptr, &hdutype, &this->m_status);
    this->check();

    if ((hdutype != ASCII_TBL) && (hdutype != BINARY_TBL))
    {
        throw runtime_error("Non-table hdu found");
    }

    long nrows;
    fits_get_num_rows(this->m_fptr, &nrows, &this->m_status);
    this->check();

    return nrows;


}

fitsfile **Fits::fptr() { return &this->m_fptr; }
int &Fits::status() { return this->m_status; }

void Fits::check(int status)
{
    if (status)
    {
        char buf[FLEN_STATUS];
        fits_get_errstatus(status, buf);
        throw runtime_error(buf);
    }
}

const string Fits::hduname()
{
    char buf[FLEN_VALUE];
    fits_read_key(this->m_fptr, TSTRING, "EXTNAME", buf, NULL, &this->m_status);
    this->check();
    return string(buf);
}

NewFits::NewFits(const string &filename)
{
    this->m_filename = filename;
    this->m_status = 0;

    fits_create_file(&this->m_fptr, filename.c_str(), &this->m_status);
    this->check();

    /* Ensure the basic keywords are there */
    long naxes[] = {0, 0};
    fits_create_img(this->m_fptr, BYTE_IMG, 0, naxes, &this->m_status);
    this->check();
}
