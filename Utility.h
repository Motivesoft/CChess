#pragma once

extern char* trim( char* string );
extern char* sanitize( char* string );

typedef void( *writeToFile )( char* format, ... );