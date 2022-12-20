#ifndef lint	/* .../sys/psn/io/mk_feature.c */
#define _AC_NAME mk_feature_c
#define _AC_MAIN "@(#) Copyright (c) 1987 Apple Computer Inc., All Rights Reserved.  {Apple version 1.3 87/11/19 18:02:25}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of mk_feature.c on 87/11/19 18:02:25";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)mk_feature.c	UniPlus VVV.2.1.1	*/
extern screen_image[];
main()
{
	extract(screen_image, "screen_feature1", 0, 0, 224, 24);
	extract(screen_image, "screen_feature2", 640-8, 0, 8, 48);
	extract(screen_image, "screen_feature3", 0, 24, 8, 24);
	extract(screen_image, "screen_feature4", 0, 48, 16, 2);
	extract(screen_image, "screen_feature5", 640-16, 48, 16, 2);
	extract(screen_image, "screen_feature6", 224, 0, 8, 24);
	extract(screen_image, "screen_feature7", 192, 24, 256, 24);
	extract(screen_image, "screen_feature8", 32, 24, 8, 24);
	extract(screen_image, "screen_feature9", 8, 480-12, 8, 12);
	extract(screen_image, "screen_feature10", 0, 480-12, 8, 12);
	extract(screen_image, "screen_feature11", 640-8, 480-12, 8, 12);
	return(0);
}


extract(d, name, x, y, len, height)
char *d, *name;
{
	unsigned char *lp;
	int i, j, inc;

	printf("\nunsigned char %s[] = {\n",name);
	inc = 0;
	for (i = 0; i < height; i++) {
		lp = (unsigned char *)(d + (y*640/8) + (x/8));
		for (j = 0; j < len; j+=(8*sizeof(unsigned char))) {
			printf("0x%x, ",*lp++);
			if (inc++ > 8) {
				inc = 0;
				printf("\n");
			}
		}
		y++;
	}
	printf("\n};\n");
}
