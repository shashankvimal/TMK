/**
* @file sema4.c
* @brief this file is used to sharing single resource between task.
* @author  SHASHANK VIMAL
* @date 3/18/2018
*/

typedef  struct sema4
{
	/*RW*/LIST_t tlist;
}sema4_t;