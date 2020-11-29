/* copied from freecom/suppl/suppl.h */

#ifdef __WATCOMC__
/* redefine struct name */
#define ffblk find_t
/* rename one of the member of that struct */
# define ff_name name

#define findfirst(pattern,buf,attrib) _dos_findfirst((pattern), (attrib)	\
	, (struct find_t*)(buf))
#define findnext(buf) _dos_findnext((struct find_t*)(buf))


#else /* __GNUC__ */

int findfirst(const char * const pattern, struct ffblk *ff, int attrib);
int findnext(struct ffblk *ff);

#endif

