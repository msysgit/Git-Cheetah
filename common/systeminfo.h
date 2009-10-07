#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H


/* git_path returns additions to the systems path variable and is used
 * by exec_program for the environment. If an implementation does not
 * want to add anything to the path it should return an empty string. An
 * error is indicated by returning NULL.
 */
const char *git_path();

/* opens a message box with ok button on this platform */
void message_box(const char *string);

/* returns true if path is a directory */
int is_path_directory(const char *path);

#endif /* SYSTEMINFO_H */
