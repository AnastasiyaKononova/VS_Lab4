#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define FILENAME "/tmp/MAIN"
#define MAX_DIR_AMOUNT 1000

int amount_dir = 0;
FILE *log_file;

struct struct_for_dir 
{
	int id;
	int empty;
	int parent_id;
	char dir_name[255];
};

void * init_do()
{
	log_file = fopen("filesystem.log", "wb");
	FILE* file = fopen(FILENAME, "wb");
	
	struct struct_for_dir ds;
	int i;

	ds.empty = 1;
	ds.parent_id = -1;	
	
	for (i = 0; i < MAX_DIR_AMOUNT; i++) 
	{
		ds.id = i;
		fwrite(&ds, sizeof(struct struct_for_dir), 1, file);
	}
	
	fclose(file);
	
	amount_dir = 0;
	void* v = NULL;
	
	return v;
}

void destroy_do(void* private_data)
{
	fflush(log_file);
	fclose(log_file);
}

int find(const char* dir, int parent)
{
	struct struct_for_dir ds;
	FILE* file = fopen(FILENAME, "rb");
	
	if (file == NULL) 
	{
		return -2;
	}
	
	memset(&ds, 0, sizeof(struct struct_for_dir));

	while(!feof(file) && fread(&ds, sizeof(struct struct_for_dir), 1, file) > 0) 
	{
		if (ds.empty == 0 && ds.parent_id == parent && strcmp(ds.dir_name, dir) == 0) 
		{
			fclose(file);
			return ds.id;
		}
	}
	
	fclose(file);
	
	return -2;
}

int find_child(struct struct_for_dir *child, int parent, int offset)
{
	FILE* file = fopen(FILENAME, "rb");
	
	if (file == NULL) 
	{
		fprintf(log_file, "--Cannot read file. Exit from find_child\n");
		fflush(log_file);
		return -2;
	}

	fseek(file, offset * sizeof(struct struct_for_dir), SEEK_SET);
	memset(child, 0, sizeof(struct struct_for_dir));
	int counter = 0;
	
	while(!feof(file) && fread(child, sizeof(struct struct_for_dir), 1, file) > 0) 
	{
		counter ++;
		if (counter % 50 == 0) 
		{
			fprintf(log_file, "Read %d\n", counter);
			fflush(log_file);
		}
	
		if (child->empty == 0 && child->parent_id == parent) 
		{
			fprintf(log_file, "Return %d\n", child->id);
			fflush(log_file);
			fclose(file);
			return child->id + 1;
		}
	}
	
	fclose(file);
	fprintf(log_file, "Return -2\n");
	fflush(log_file);
	
	return -2;
}

int add(struct struct_for_dir *dir)
{	
	FILE* file = fopen(FILENAME, "rb+");
	struct struct_for_dir ds;
	
	memset(&ds, 0, sizeof(struct struct_for_dir));
	
	while(!feof(file) && fread(&ds, sizeof(struct struct_for_dir), 1, file) > 0) 
	{
		if (ds.empty) 
		{
			fseek(file, ds.id * sizeof(struct struct_for_dir), SEEK_SET);
			dir->id = ds.id;
			fwrite(dir, sizeof(struct struct_for_dir), 1, file);
			fprintf(log_file, "Create dir with id: %d\n", dir->id);
			fclose(file);
			amount_dir = amount_dir + 1;
			return ds.id;
		}
	}
	
	fprintf(log_file, "Cannot create dir \n");
	
	fflush(log_file);
	fclose(file);
	return -2;
}

int delete(int id)
{
	FILE* file = fopen(FILENAME, "rb+");
	
	if (file == NULL) 
	{
		fprintf(log_file, "Cannot read file. Exit from find_child\n");
		fflush(log_file);
		return -2;
	}
	
	struct struct_for_dir buf;
	
	buf.id = id;
	buf.empty = 1;
	
	fseek(file, id * sizeof(struct struct_for_dir), SEEK_SET);
	fwrite(&buf, sizeof(struct struct_for_dir), 1, file);
	fclose(file);
	
	fprintf(log_file, "Return 0\n");	
	fflush(log_file);
	
	amount_dir = amount_dir - 1;
	
	return 0;
}

int ren_ame(struct struct_for_dir *dir)
{
	FILE* file = fopen(FILENAME, "rb+");
	
	if (file == NULL) 
	{
		fprintf(log_file, "Cannot read file. Exit from find_child\n");
		fflush(log_file);
		return -2;
	}
	
	dir->empty = 0;	
	fseek(file, dir->id * sizeof(struct struct_for_dir), SEEK_SET);
	fwrite(dir, sizeof(struct struct_for_dir), 1, file);
	fclose(file);
	
	fprintf(log_file, "	Return 0\n");
	
	fflush(log_file);
	
	return 0;
}

int find_by_path(const char* path)
{
	char subdir[255];
	char *endp = NULL;
	int start = 1, end = 0, len = strlen(path), parent = -1, id = -1;

	if (strcmp("/", path) == 0) 
	{
		return -1;
	} 
	else 
	{
		while(start <= len - 1) 
		{
			endp = strchr(path + start, '/');
			if(endp == NULL) 
			{
				memset(subdir, 0, 255);
				strcpy(subdir, path + start);
				end = len - start;
				
			} 
			else 
			{
				end = endp - path - start;
				memset(subdir, 0, 255);
				strncpy(subdir, path + start, end);
			}
			id = find(subdir, parent);
			if (id < -1) 
			{
				return -2;
			}
			parent = id;
			start = start + end + 1;
		}	
	}
	
	return id;
}

int find_by_parent(const char* path, struct struct_for_dir *ds)
{
	char subdir[255];
	char *endp = NULL;
	int start = 1, end = 0, len = strlen(path), parent = -1, id = -1;
	
	if (strcmp("/", path) == 0) 
	{
		fprintf(log_file, "Exit from mkdir 1: return 0\n");
		return -1;
	} 
	else 
	{
		while(start <= len - 1) 
		{
			endp = strchr(path + start, '/');
			if(endp == NULL) 
			{
				memset(subdir, 0, 255);
				strcpy(subdir, path + start);
				break;
			}
			end = endp - path - start;
			memset(subdir, 0, 255);
			strncpy(subdir, path + start, end);
			id = find(subdir, parent);
			if (id < -1) 
			{
				return -2;
			}
			parent = id;
			start = start + end + 1;
		}	
	}
	
	ds->id = -2;
	ds->parent_id = parent;
	ds->empty = 0;
	
	memset(ds->dir_name, 0, 255);
	strcpy(ds->dir_name, subdir);
	
	return 0;
}


static int getattr_do(const char *path, struct stat *stbuf) 
{
  	fprintf(log_file, "----Enter to getattr\n");
	
	int len = strlen(path), id = -1;
	memset(stbuf, 0, sizeof(struct stat));
	
	fprintf(log_file, "-----getattr path: %s\n", path);
	fprintf(log_file, "path len: %d\n", len);
	
	fflush(log_file);
	id = find_by_path(path);
	
	if (id < -1) 
	{
		return -ENOENT;
	}
	
	if (id == -1) 
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		fprintf(log_file, "----Exit from getattr1: return 0\n");
		fflush(log_file);
		return 0;
	} 
	
	stbuf->st_mode = S_IFDIR | 0777;
    	stbuf->st_nlink = 2;
	
	fprintf(log_file, "----Exit from getattr2: return 0\n");
	fflush(log_file);
	
	return 0;
}


int rmdir_do(const char* path)
{
	struct struct_for_dir ds;
	int id = -1;
	
	fflush(log_file);
	id = find_by_path(path);
	
	if (id < -1) 
	{
		return -ENOENT;
	}
	
	if (id == -1) 
	{
		return -EBUSY;
	}
	
	if (find_child(&ds, id, 0) > -1) 
	{
    		return -ENOTEMPTY;
    	}
	
	delete(id);
	
	return 0;
}

int mkdir_do(const char* path, mode_t mode)
{
	if (amount_dir == MAX_DIR_AMOUNT) 
	{
		return -ENOSPC;
	}
	
	fprintf(log_file, "----Enter to mkdir\n");
	fflush(log_file);
	
	struct struct_for_dir ds;
	int len = strlen(path), res = 0;
	
	fprintf(log_file, "------mkdir path: %s\n", path);
	fprintf(log_file, "path len: %d\n", len);
	fflush(log_file);
	res = find_by_parent(path, &ds);
	if (res == -1) 
	{
		return 0;
	}
	if (res < -1) 
	{
		return -ENOENT;
	}
	fprintf(log_file, "---Add: %d\n", add(&ds));
	fprintf(log_file, "----Exit from mkdir 2: return 0\n");
	fflush(log_file);
	return 0;
}

int rename_do(const char* from, const char* to)
{
	struct struct_for_dir ds;
	int from_id = -2, to_id = -2, res = 0;
	
	if (strcmp(from, to) == 0) 
	{
		return 0;
	}
	
	from_id = find_by_path(from);
	
	if (from_id == -1) 
	{
		return -EACCES;
	}
	
	if (from_id < -1) 
	{
		return -ENOENT;
	}
	
	to_id = find_by_path(to);
	
	if (to_id == -1) 
	{
		return -EBUSY;
	}
	if (to_id > -1 && find_child(&ds, to_id, 0) > -1) 
	{
		return -ENOTEMPTY;
	}
	if (strstr(from, to) != NULL) 
	{
		return -EINVAL;
	}
	
	memset(&ds, 0, sizeof(struct struct_for_dir));
	res = find_by_parent(to, &ds);
	
	if (res < -1) 
	{
		return -ENOENT;
	}
	
	ds.id = from_id;
	
	if (to_id > -1) 
	{
		delete(to_id);
	}
	
	ren_ame(&ds);
	
	return 0;
}

int readdir_do(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	struct struct_for_dir ds;
	int id = -1, ofst = 0;
	id = find_by_path(path);
	if (id < -1) 
	{
		return -ENOENT;
	}
	filler(buf, ".", NULL, 0);
    	filler(buf, "..", NULL, 0);
    	while ((ofst = find_child(&ds, id, ofst)) > -1) 
	{
    		fflush(log_file);
    		filler(buf, ds.dir_name, NULL, 0);
    	}
    	fflush(log_file);
	return 0;
}

static struct fuse_operations my_sys_operations = 
{
	.getattr = getattr_do,
	.init = init_do,
	.rmdir = rmdir_do,
	.mkdir = mkdir_do,
	.readdir = readdir_do,
	.rename = rename_do,
	.destroy = destroy_do
};

int main(int argc, char *argv[])
{
  	return fuse_main(argc, argv, &my_sys_operations, NULL);
}
