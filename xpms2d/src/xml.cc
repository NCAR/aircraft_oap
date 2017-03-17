/* I'm not ready to drag a whole package into xpms2d at this time.  So here
 * are a couple routines to parse and element and attribute.
 */

#include <string>
#include <cstring>

/**
 * Given a single line of XML with an element, peel out the value.
 */
std::string XMLgetElementValue(const char s[])
{
  char *start, *end;
  start = std::strchr(const_cast<char *>(s), '>') + 1;
  end = std::strchr(start, '<') - 1;

  int len = end-start+1;
  char output[1024];
  std::memcpy(output, start, len);
  output[len] = '\0';

  std::string result(output);
  return result;
}

/**
 * Given a single line of XML with attributes, peel one out.
 */
std::string XMLgetAttributeValue(const char s[], const char target[])
{
  char *start, *end;
  start = std::strstr(const_cast<char *>(s), target);
  start = std::strchr(start, '\"') + 1;
  end = std::strchr(start, '\"') - 1;

  int len = end-start+1;
  char output[1024];
  std::memcpy(output, start, len);
  output[len] = '\0';

  std::string result(output);
  return result;
}

