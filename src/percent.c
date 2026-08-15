/* Percent-encoded URL and URI surfaces.
   Copyright © 2026 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Recode Library; see the file `COPYING.LIB'.
   If not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"
#include "common.h"
#include "decsteps.h"

static bool
is_unreserved (int character)
{
  return (character >= 'A' && character <= 'Z')
    || (character >= 'a' && character <= 'z')
    || (character >= '0' && character <= '9')
    || character == '-'
    || character == '.'
    || character == '_'
    || character == '~';
}

static int
hex_value (int character)
{
  if (character >= '0' && character <= '9')
    return character - '0';
  if (character >= 'A' && character <= 'F')
    return character - 'A' + 10;
  if (character >= 'a' && character <= 'f')
    return character - 'a' + 10;
  return -1;
}

static void
put_percent_encoded (int character, RECODE_SUBTASK subtask)
{
  static const char hex[] = "0123456789ABCDEF";

  recode_put_byte ('%', subtask);
  recode_put_byte (hex[(character >> 4) & BIT_MASK (4)], subtask);
  recode_put_byte (hex[character & BIT_MASK (4)], subtask);
}

static bool
transform_data_percent (RECODE_SUBTASK subtask, bool space_as_plus)
{
  int character;

  while (character = recode_get_byte (subtask), character != EOF)
    if (is_unreserved (character))
      recode_put_byte (character, subtask);
    else if (space_as_plus && character == ' ')
      recode_put_byte ('+', subtask);
    else
      put_percent_encoded (character, subtask);

  SUBTASK_RETURN (subtask);
}

static bool
transform_percent_data (RECODE_SUBTASK subtask, bool plus_as_space)
{
  int character = recode_get_byte (subtask);

  while (character != EOF)
    if (character == '%')
      {
        int high = recode_get_byte (subtask);
        int high_value = hex_value (high);

        if (high_value < 0)
          {
            RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
            character = high;
          }
        else
          {
            int low = recode_get_byte (subtask);
            int low_value = hex_value (low);

            if (low_value < 0)
              {
                RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
                character = low;
              }
            else
              {
                int value = high_value << 4 | low_value;

                recode_put_byte (value, subtask);
                character = recode_get_byte (subtask);
              }
          }
      }
    else
      {
        if (plus_as_space && character == '+')
          recode_put_byte (' ', subtask);
        else
          recode_put_byte (character, subtask);
        character = recode_get_byte (subtask);
      }

  SUBTASK_RETURN (subtask);
}

static bool
transform_data_url (RECODE_SUBTASK subtask)
{
  return transform_data_percent (subtask, true);
}

static bool
transform_url_data (RECODE_SUBTASK subtask)
{
  return transform_percent_data (subtask, true);
}

static bool
transform_data_uri (RECODE_SUBTASK subtask)
{
  return transform_data_percent (subtask, false);
}

static bool
transform_uri_data (RECODE_SUBTASK subtask)
{
  return transform_percent_data (subtask, false);
}

bool
module_percent (RECODE_OUTER outer)
{
  return
    recode_declare_single (outer, "data", "URL",
			   outer->quality_variable_to_variable,
			   NULL, transform_data_url)
    && recode_declare_single (outer, "URL", "data",
			      outer->quality_variable_to_variable,
			      NULL, transform_url_data)
    && recode_declare_single (outer, "data", "URI",
			      outer->quality_variable_to_variable,
			      NULL, transform_data_uri)
    && recode_declare_single (outer, "URI", "data",
			      outer->quality_variable_to_variable,
			      NULL, transform_uri_data);
}

void
delmodule_percent (_GL_UNUSED RECODE_OUTER outer)
{
}
