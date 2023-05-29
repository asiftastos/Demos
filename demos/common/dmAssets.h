#pragma once

char* loadFileText(const char* filename, int *length);
void freeText(char* text);

#ifdef DM_ASSETS_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>

char* loadFileText(const char* filename, int *length)
{
	char* text = NULL;

	if (filename != NULL)
	{
		FILE* file = fopen(filename, "rt");

		if (file != NULL)
		{
			fseek(file, 0, SEEK_END);
			int size = ftell(file);
			fseek(file, 0, SEEK_SET);

			if (size > 0)
			{
				text = (char*)calloc((size + 1), sizeof(char));
				unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

				if (count < (unsigned int)size)
				{
					text = realloc(text, count + 1);
					*length = count;
				}
				else
				{
					*length = size;
				}

				text[count] = '\0';
			}

			fclose(file);
		}
	}

	return text;
}

void freeText(char* text)
{
	if (text)
	{
		free(text);
		text = NULL;
	}
}

#endif // DM_ASSETS_IMPLEMENTATION
