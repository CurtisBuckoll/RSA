// ============================================================================
// RSA.cpp
// This program is not large, and all of the functionality lives inside this 
// file - maybe one day I'll wrap this inside its own class.
//
// by Curtis Buckoll
// ============================================================================

#include "stdafx.h"
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>


// ============================================================================
// 

#define PRIME_DIST_LO 1001
#define PRIME_DIST_HI 10001

// ============================================================================
// Error Handling
struct Error
{
    // -------------------------------------------------------------
    //
    static void FatalError( std::string e );
};

// -------------------------------------------------------------
//
void Error::FatalError( std::string e )
{
    std::cout << "Fatal Error: " + e + ". Exiting program." << std::endl;
    exit( -1 );
}


// ============================================================================
// OrderedPair
struct OrderedPair {

    // -------------------------------------------------------------
    //
    OrderedPair()
    {
        a_ = b_ = 0;
    }

    // -------------------------------------------------------------
    //
    OrderedPair( int a, int b )
    {
        a_ = a;
        b_ = b;
    }

    // -------------------------------------------------------------
    //
    void Print()
    {
        std::cout << "(" << a_ << ", " << b_ << ")" << std::endl;
    }

    // -------------------------------------------------------------
    //
    const OrderedPair& operator=( const OrderedPair& rhs )
    {
        a_ = rhs.a_;
        b_ = rhs.b_;
        return *this;
    }

    int a_;
    int b_;
};


// ============================================================================
// Program Functions

// ============================================================================
// Returns the coefficients (x,y) of (a,b) in ax + by = d such that d is the
// gcd of a and b.
OrderedPair bezout( int a, int b )
{
    if( a <= 0 || b <= 0 )
    {
        Error::FatalError( "Bad coefficients: a: " + std::to_string( a ) + " b: " + std::to_string( b ) );
    }

    // In form (prevX, x)
    OrderedPair r_pair = OrderedPair( a, b );
    OrderedPair s_pair = OrderedPair( 1, 0 );
    OrderedPair t_pair = OrderedPair( 0, 1 );

    while( r_pair.b_ != 0 )
    {
        int quotient = r_pair.a_ / r_pair.b_; // Integer division

        r_pair = OrderedPair( r_pair.b_, r_pair.a_ - quotient * r_pair.b_ );
        s_pair = OrderedPair( s_pair.b_, s_pair.a_ - quotient * s_pair.b_ );
        t_pair = OrderedPair( t_pair.b_, t_pair.a_ - quotient * t_pair.b_ );
    }

    return OrderedPair( s_pair.a_, t_pair.a_ );
}

// ============================================================================
// Returns gcd(a,b) where a, b, gcd(a,b) >= 0
unsigned int GCD( unsigned int a, unsigned int b )
{
    if( a < b ) { std::swap( a, b ); }

    while( b != 0 )
    {
        unsigned int r = a % b;
        a = b;
        b = r;
    }

    return a;
}

// ============================================================================
// Returns the least positive residue of b^x (mod n)
unsigned int modularExponention( unsigned int b, 
                                 unsigned int x, 
                                 unsigned int n )
{
    // This is max size (in bits) of the exponent x.
    static const size_t EXPSZ      = 64;

    unsigned long long  curr_power = b;
    unsigned long long  result     = 1;

    // Accumulate the result by efficient modular exponentiation
    // algorithm.
    for( unsigned int i = 0; i < EXPSZ; ++i )
    {
        if( 0x1 & x )
        {
            result = ( result * curr_power ) % n;
        }

        curr_power = ( curr_power * curr_power ) % n;
        x          = x >> 1;
    }

    // Since the modulus n fits into an unsigned int, the
    // result of this operation certainly should.
    return static_cast<unsigned int>( result );
}

// ============================================================================
// Check the first bases b \in {2,...,100}
bool isPrime( unsigned int n )
{
    if( n < 1 )
    {
        Error::FatalError( "Modulus is not a positive integer: " + std::to_string( n ) );
    }

    for( unsigned int base = 2; base < 100; base++ )
    {
        unsigned int lhs = modularExponention( base, n, n );

        if( ( lhs % n ) != ( base % n ) )
        {
            return false;
        }
    }

    return true;
}

// ============================================================================
// Return a prime (OR pseudoprime) on distribution interval
unsigned int getPrime()
{
    std::random_device device;
    std::mt19937 generator( device() );
    std::uniform_int_distribution<int> distribution( PRIME_DIST_LO, PRIME_DIST_HI );

    int p = distribution( generator );
    if( p % 2 == 0 ) { ++p; }
    while( !isPrime( p ) ) { p += 2; }

    return p;
}

// ============================================================================
//
unsigned int getEulerPhi( unsigned int p, 
                          unsigned int q )
{
    return ( p - 1 ) * ( q - 1 );
}

// ============================================================================
// Assumption: e is a least positive residue of n
unsigned int inverseModN( unsigned int e, 
                          unsigned int n )
{
    if( n <= 0 )
    {
        Error::FatalError( "Bad modulus: " + std::to_string( n ) );
        exit( -1 );
    }

    OrderedPair pair = bezout( n, e );

    int inverse = pair.b_;
    while( inverse < 0 ) { inverse += n; }

    return static_cast<unsigned int>( inverse );
}

// ============================================================================
// 
void generateExponentFactors( unsigned int& e, 
                              unsigned int& d, 
                              unsigned int euler_phi )
{
    std::random_device device;
    std::mt19937 generator( device() );
    std::uniform_int_distribution<int> distribution( 2, euler_phi - 1 );

    e = distribution( generator );
    while( GCD( e, euler_phi ) != 1 ) { e++; }

    d = inverseModN( e, euler_phi );
}

// ============================================================================
// Take string of input message and return an encrypted vector of integers
std::vector<int> encrypt( const std::string& input, 
                          unsigned int e, 
                          unsigned int n )
{
    std::vector<int> encryptedData;

    int i = input.size() - 1;
    for( i; i > 0; i -= 2 )
    {
        short upper = input[i - 1];
        short lower = input[i];
        int data = upper * 1000 + lower;
        int encrypted = modularExponention( upper * 1000 + lower, e, n );
        std::vector<int>::iterator it;
        it = encryptedData.begin();
        encryptedData.insert( it, encrypted );
    }
    for( i; i >= 0; i-- )
    {
        int data = input[i];
        int encrypted = modularExponention( data, e, n );
        std::vector<int>::iterator it;
        it = encryptedData.begin();
        encryptedData.insert( it, encrypted );
    }

    return encryptedData;
}

// ============================================================================
// Take vector of integers as encrypted input and return a string orignal 
// (decrypted) message
std::string decrypt( const std::vector<int>& encrypted_input, 
                     unsigned int d, 
                     unsigned int n )
{
    std::string decryptedData;

    for( unsigned int i = 0; i < encrypted_input.size(); i++ )
    {
        int decrypted = modularExponention( encrypted_input[i], d, n );

        std::string decryptedSubString = std::to_string( decrypted );
        while( decryptedSubString.size() < 6 )
        {
            decryptedSubString.insert( 0, "0" );
        }

        decryptedData.append( "XX" );
        decryptedData[decryptedData.size() - 2] = static_cast<char>( stoi( decryptedSubString.substr( 0, 3 ) ) );
        decryptedData[decryptedData.size() - 1] = static_cast<char>( stoi( decryptedSubString.substr( 3, 3 ) ) );
    }

    return decryptedData;
}

// ============================================================================
// Convert encrypted message (vector of integers) and print ASCII equivalent
void printEncryptedMessage( std::vector<int> message )
{
    for( unsigned int i = 0; i < message.size(); i++ )
    {
        std::string token = std::to_string( message[i] );

        unsigned int k = 0;
        for( k; k < token.size() - 1; k += 2 )
        {
            std::cout << static_cast<char>( 30 + std::stoi( token.substr( k, 2 ) ) );
        }
        for( k; k < token.size(); k++ )
        {
            std::cout << static_cast<char>( 30 + std::stoi( token.substr( k, 1 ) ) );
        }
    }

    std::cout << std::endl;
}

// ============================================================================
// Entry
int main()
{
    // Set up.
    unsigned int p = getPrime();
    unsigned int q = getPrime();
    while( q == p )
    {
        q = getPrime();
    }
    unsigned int  n = p * q;
    unsigned int e, d;
    generateExponentFactors( e, d, getEulerPhi( p, q ) );

    // Interaction.
    std::string data;
    std::cout << "(Your decryption key is: " << d << ")\n" << std::endl;
    std::cout << "Enter a message:" << std::endl;
    std::getline( std::cin, data );

    std::cout << std::endl;

    std::cout << "Encrypted message: " << std::endl;
    std::vector<int> encryptedMessage = encrypt( data, e, n );
    printEncryptedMessage( encryptedMessage );

    std::cout << "\nEnter decryption key or 0 to exit: ";

    int key = 1;
    std::string key_input;

    while( key != 0 )
    {
        // Get key input from user.
        while( true )
        {
            std::cin >> key_input;

            try
            {
                key = std::stoi( key_input );
                break;
            }
            catch( ... )
            {
                std::cout << "Input must be integral and within 32-bit unsigned range. Try again: ";
            }
        }

        std::cout << "Decrypted Message: " << decrypt( encryptedMessage, key, n ) << std::endl << std::endl;
        std::cout << "Try a new key: ";
    }

    return 0;
}
