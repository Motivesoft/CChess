#pragma once

/// <summary>
/// Trims spaces from the front and end of a string.
/// This method may modify the input string.
/// </summary>
/// <param name="string">the input string</param>
/// <returns>the trimmed string</returns>
char* trim( char* string );

/// <summary>
/// Remove double spaces, tabs etc from either end and within a string
/// This method may modify the input string.
/// </summary>
/// <param name="string">the input string</param>
/// <returns>the trimmed string</returns>
char* sanitize( char* string );

/// <summary>
/// Take the input line and return pointers to the first word, and to the rest of the string.
/// This allows us to split a command such as "go depth 3" into its keyword and its arguments.
/// During processing, the string is trimmed and sanitized to eliminated unwanted spaces, tabs etc.
/// This method may modify the input string
/// </summary>
/// <param name="line">the input line</param>
/// <param name="command">pointer to the command</param>
/// <param name="arguments">pointer to the arguments</param>
/// <returns>true if arguments is not expected to be zero length</returns>
bool spliterate( char* line, char** command, char** arguments );

typedef void( *WriteToFile )( char* format, ... );
