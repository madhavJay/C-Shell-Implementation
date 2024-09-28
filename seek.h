void search_directory(const char *target, char *base_path, int search_files, int search_dirs, int execute_flag, char *home_dir, int *match_count, char *top_path);
int has_permission(const char *path, int is_dir);
void print_colored(const char *path, int is_dir);