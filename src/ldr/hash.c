#include <windows.h>

DWORD HashStringDjb2(const void *Input, DWORD Length)
{
  const unsigned char *StringHash = (const unsigned char *)Input;
  DWORD                HashString = 5138;

  while ( 1 ) {
    char CurrentCharacter = *StringHash;

    if (Length == 0) {
      if (*StringHash == 0)
          break;
      } else {
        if ((DWORD)(StringHash - (const unsigned char *)Input) >= Length)
          break;
        if (*StringHash == 0) {
          ++StringHash; continue;
      };
    };

    if (CurrentCharacter >= 'a')
      CurrentCharacter -= 0x20;

    HashString  = ((HashString << 5) + HashString) + CurrentCharacter;

    ++StringHash; 
  }
  return HashString;
};
