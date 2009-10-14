#ifndef GIT_CHEETAH_UTIL_H
#define GIT_CHEETAH_UTIL_H

static inline int parse_and_remove_shortcuts(char *name)
{
	int i,j;
	char key = 0;
	for (i=0,j=0; name[i] && name[j]; i++,j++) {
		if(!key && name[j] == '&') {
			key = name[j+1];
			j++;
		}
		name[i] = name[j];
	}
	name[i] = '\0';
	return key;
}

#endif /* GIT_CHEETAH_UTIL_H */
