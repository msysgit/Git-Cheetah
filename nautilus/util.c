/* util.c
 *
 * Nautilus specific implementations of platform dependent functions for
 * git-cheetah
 *
 * Copyright (C) Heiko Voigt, 2009
 *
 */

#include "../common/git-compat-util.h"
#include "../common/systeminfo.h"

const char *git_path()
{
	return "";
}

void message_box(const char *string)
{
}
