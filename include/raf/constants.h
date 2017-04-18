/* constants.h  -- simple constants used frequently
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifndef __cplusplus
typedef unsigned char   bool;
#endif

#define ERROR 		-1
#define NONE		-1
#define BYTE_LEN	8
#define BYTESPERWORD 	4
#define BITSPERWORD	(BYTESPERWORD * BYTE_LEN)
#define NAMLEN		13

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

#ifndef ERR
#define OK		(0)
#define ERR		(-1)
#endif

#define COMMENT		'#'	/* Comment character for textfiles	*/
 
#define FULL_CIRCLE		360.0
#define TWO_PI        		(2.0 * 3.14159265358979323846)

#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

#ifndef MAX
#define MAX(x,y) (x>y?x:y)
#endif

#ifndef MIN
#define MIN(x,y) (x>y?y:x)
#endif

#define odd(x) ((x%2)?TRUE:FALSE)

#endif
