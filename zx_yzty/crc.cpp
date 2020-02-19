#include "crc.h"

unsigned short CRC_16( unsigned char * puchMsg, unsigned short usDataLen)
{
    unsigned char uchCRCHi;
    unsigned char uchCRCLo;
    unsigned uIndex;
    uchCRCHi = 0xFF;
    uchCRCLo = 0xFF;
    while ( usDataLen-- )
    {
        uIndex = uchCRCHi ^ (unsigned char)( *puchMsg++ );
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }
    return ( uchCRCHi  << 8 | uchCRCLo );
}
