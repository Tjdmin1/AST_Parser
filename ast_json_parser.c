#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wvoid-pointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wformat"
#include <stdio.h>
#include <stdlib.h>
#include "json_c.c"
#include <stdbool.h>
#define FILE_NAME_LEN 50

// 함수 선언
char *input_file_name();
void all_free(char *file_name, json_value json_text);
FILE *open_file(char *file_name);
int get_file_size(FILE *fp);
char *get_file_text(FILE *fp, int file_size);
json_value text_to_json(char *text);
void get_function_info(json_value json_text);
void get_function_parameters_info(json_value json_text);
void get_if_count(json_value json_text);
int subdata(json_value data, int count);
void close_file(FILE *fp);

int main(void){      
	char *file_name = input_file_name();
	FILE *ast_file = open_file(file_name);
	long file_size = get_file_size(ast_file);

	if(file_size == 0){
		printf("File Size : %d\n파일에 내용이 없습니다.", 0);
		close_file(ast_file);
		free(file_name);
		exit(1);
	}

	printf("[ File Size : %d Byte ]\n", file_size);

	char *text = get_file_text(ast_file, file_size);
	json_value json_text = text_to_json(text);

	if(json_text.value == 0){
		printf("파일 내용이 JSON 형식이 아닙니다.\n");
		close_file(ast_file);
		all_free(file_name, json_text);
		exit(1);
	}

	get_function_info(json_text);
	get_function_parameters_info(json_text);
	get_if_count(json_text);

	printf("====================================================");
	
	close_file(ast_file);
	all_free(file_name, json_text);
}

char *input_file_name(){
	printf("\n      [WHS] C-LANG AST PARSE ( Team.네얼간이 )\n====================================================\n");
	char *file_name = (char *)malloc(sizeof(char) * FILE_NAME_LEN);
	printf("C-Lang AST File을 입력해주세요 : ");
	fgets(file_name, FILE_NAME_LEN, stdin);
	// 개행 문자 제거
	file_name[strlen(file_name) - 1] = '\0';
	return file_name;
}

void all_free(char *file_name, json_value json_text){
	free(file_name);
	json_free(json_text);
}

FILE *open_file(char *file_name){
	FILE *fp = fopen(file_name, "r");

	// If can't find File
	if(fp == NULL){
		perror("----------------------------------------------------\nFile Error : 파일 이름이 잘못되었거나 파일이 존재하지 않습니다.\nError ");
		printf("\n");
		free(file_name);
        exit(1);
	}
	
	printf("----------------------------------------------------\n파일 읽기 성공\n");
	printf("-----------------[File Information]-----------------\n");
	printf("[ File Name : %s ]\n", file_name);
	return fp;
}

int get_file_size(FILE *fp){
    long file_size;

	fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
	
	if(file_size == 0){
		return 0;
	}

	return file_size;
}

char *get_file_text(FILE *fp, int file_size){
	char *buffer = (char *)malloc(sizeof(char) * file_size);
	size_t read_size = fread(buffer, 1, file_size, fp);

	return buffer;
}

json_value text_to_json(char *text){
	json_value str = json_create(text);
	return str;
}

void get_function_info(json_value json_text){
	printf("--------------[Functions Information]---------------\n");
	printf(" r#** Functions in header files are not included **#\n");
	json_value ext = json_get(json_text, "ext");
	int count = 0;
	for(int i = 0; i < json_len(ext); i++){
		char *check_func = json_get_string(json_get(json_get(json_get(ext, i)), "_nodetype"));
		if(!strcmp(check_func, "FuncDef")){
			json_value data = json_get(json_get(ext, i), "decl");
			printf("[ Function Name : %s |", json_get_string(json_get(data, "name")));

			if(!strcmp("PtrDecl", json_get_string(json_get(json_get(json_get(data, "type"), "type"), "_nodetype")))){
				printf(" Return Type : %s* ]\n", json_get_string(json_get(json_get(json_get(json_get(json_get(json_get(data, "type"), "type"), "type"), "type"), "names"), 0))); 
			}
			else{
				printf(" Return Type : %s ]\n", json_get_string(json_get(json_get(json_get(json_get(json_get(data, "type"), "type"), "type"), "names"), 0)));
			}
			count ++;
		}
	}
	printf("[ Number of Functions : %d ]\n", count);
}

void get_function_parameters_info(json_value json_text){
	printf("----------[Function Parameter Information]----------\n");
	json_value ext = json_get(json_text, "ext");
	for(int i = 0; i < json_len(ext); i++){
		char *check_func = json_get_string(json_get(json_get(json_get(ext, i)), "_nodetype"));
		if(!strcmp(check_func, "FuncDef")){
			json_value data = json_get(json_get(ext, i), "decl");
			printf("#-------[ Function Name : %s ]-------#\n", json_get_string(json_get(data, "name")));
			if(json_get_type(json_get(json_get(data, "type"), "args")) == JSON_NULL){
				printf("{ No Parameters }\n");
			}
			else{
				data = json_get(json_get(json_get(json_get(json_get(ext, i), "decl"), "type"), "args"), "params");
				for(int j = 0; j < json_len(data); j++){
					if(!strcmp("PtrDecl", json_get_string(json_get(json_get(json_get(data, j), "type"), "_nodetype")))){
						if(!strcmp("void", json_get_string(json_get(json_get(json_get(json_get(json_get(json_get(data, j), "type"), "type"), "type"), "names"), 0)))){
							printf("{ No Parameters }\n");
						}else{
							printf("[ Parameter Name : %s | Type : %s* ]\n", json_get_string(json_get(json_get(json_get(data,j), "type"), "type"), "declname"), json_get_string(json_get(json_get(json_get(json_get(json_get(json_get(data, j), "type"), "type"), "type"), "names"), 0)));
						}
					}else{
						if(!strcmp("void", json_get_string(json_get(json_get(json_get(json_get(json_get(data, j), "type"), "type"), "names"), 0)))){
							printf("{ No Parameters }\n");
						}else{
							printf("[ Parameter Name : %s | Type : %s ]\n", json_get_string(json_get(json_get(data,j), "type"), "declname"),json_get_string(json_get(json_get(json_get(json_get(json_get(data, j), "type"), "type"), "names"), 0)));
						}
					}
				}
			}
			if(i < json_len(ext)-1){
				printf("\n");
			}
		}
	}
}

void get_if_count(json_value json_text){
	json_value ext = json_get(json_text, "ext");
	printf("---------------[Function IF Counting]---------------\n");
	int total_count = 0;
	for(int i = 0; i < json_len(ext); i++){
		char *check_func = json_get_string(json_get(json_get(json_get(ext, i)), "_nodetype"));
		if(!strcmp(check_func, "FuncDef")){
			json_value data = json_get(json_get(ext, i), "decl");
			printf("#-------[ Function Name : %s ]-------#\n", json_get_string(json_get(data, "name")));
			if(json_get_type(json_get(json_get(json_get(ext, i), "body"), "block_items")) == JSON_NULL){
				printf("| IF Conditional Statement : 없음 |\n");
			}else{
				json_value data = json_get(json_get(json_get(ext, i), "body"), "block_items");
				int count = subdata(data, 0);
				total_count = total_count + count;
				if(count == 0){
					printf("| IF Conditional Statement : 없음 |\n");
				}else{
					printf("| If conditional statement : %d개 |\n", count);
				}
			}
			printf("\n");
		}
	}
	if(total_count == 0){
		printf("[ Total If conditional statement : 없음 ]\n");
	}
	else{
		printf("[ Total If conditional statement : %d개 ]\n", total_count);
	}
}

int subdata(json_value data, int count){
	for(int j = 0; j < json_len(data); j++){
		if(json_get_type(json_get(data, j)) == 0x10 || json_get_type(json_get(data, j)) == 0x8){
			count = subdata(json_get(data, j), count);
		}else{
			if(json_get_type(json_get(data, j)) == 0x2){
				if(!strcmp("If", json_get_string(json_get(data, j)))){
					count++;
				}
			}
		}
	}
	return count;
}

void close_file(FILE *fp){
	if(fclose(fp) == 0){
		return;
	}
	else{
		perror("File Pointer Error ");
		exit(1);
	}
}

#pragma GCC diagnostic pop

