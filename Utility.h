#pragma once

char* trim( char* string );
char* sanitize( char* string );

typedef void( *WriteToFile )( char* format, ... );
