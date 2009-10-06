#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H


/* git_path returns additions to the systems path variable and is used
 * by exec_program for the environment. If an implementation does not
 * want to add anything to the path it should return an empty string. An
 * error is indicated by returning NULL.
 */
const char *git_path();

#endif /* SYSTEMINFO_H */
