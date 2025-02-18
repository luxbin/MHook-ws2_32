// constants.h

#ifndef CONSTANTS_H
#define CONSTANTS_H


#ifdef _PACK
#define ENCRYPTED_STRING(str) VMProtectDecryptStringA(str)
#else
#define ENCRYPTED_STRING(str) str
#endif

#ifdef _PACK
#define ENCRYPTED_STRINGW(str) VMProtectDecryptStringW(str)
#else
#define ENCRYPTED_STRINGW(str) str
#endif

#endif // CONSTANTS_H