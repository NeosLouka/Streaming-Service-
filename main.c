
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "streaming_service.h"

/* Maximum input line size */
#define MAX_LINE 1024

/* 
 * Uncomment the following line to
 * enable debugging prints
 * or comment to disable it
 */
/* #define DEBUG */
#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...)
#endif /* DEBUG */


void init_structures(void)
{
	UsersList = NULL;
	NewMoviesList = NULL;
	int i ;
	for(i = 0 ; i<6; i++){
		categoriesArray[i] = NULL;
	}

	/*
	 * TODO: Initialize your
	 * global structures here,
	 * i.e., the user list (and sentinel
	 * node), new releases list, category
	 * table
	 */
}

void destroy_structures(void)
{
	/*
	 * TODO: For a bonus
	 * empty all lists and stacks
	 * and free all memory associated
	 * with list/stack nodes here
	 */
}

int main(int argc, char *argv[])
{
	FILE *event_file;
	char line_buffer[MAX_LINE];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	event_file = fopen(argv[1], "r");
	if (!event_file) {
		perror("fopen error for event file open");
		exit(EXIT_FAILURE);
	}

	init_structures();
	while (fgets(line_buffer, MAX_LINE, event_file)) {
		char *trimmed_line;
		char event;
		int uid;
		unsigned mid, year;
		movieCategory_t category1, category2;
		/*
		 * First trim any whitespace
		 * leading the line.
		 */
		trimmed_line = line_buffer;
		while (trimmed_line && isspace(*trimmed_line))
			trimmed_line++;
		if (!trimmed_line)
			continue;
		/* 
		 * Find the event,
		 * or comment starting with #
		 */
		if (sscanf(trimmed_line, "%c", &event) != 1) {
			fprintf(stderr, "Could not parse event type out of input line:\n\t%s",
					trimmed_line);
			fclose(event_file);
			exit(EXIT_FAILURE);
		}

		switch (event) {
			/* Comment, ignore this line */
			case '#':
				break;
			case 'R':
				if (sscanf(trimmed_line, "R %d", &uid) != 1) {
					fprintf(stderr, "Event R parsing error\n");
					break;
				}
				register_user(uid);
				break;
			case 'U':
				if (sscanf(trimmed_line, "U %d", &uid) != 1) {
					fprintf(stderr, "Event U parsing error\n");
					break;
				}
				unregister_user(uid);
				break;
			case 'A':
				if (sscanf(trimmed_line, "A %u %d %u", &mid, &category1,
							&year) != 3) {
					fprintf(stderr, "Event A parsing error\n");
					break;
				}
				add_new_movie(mid, category1, year);
				break;
			case 'D':
				distribute_new_movies();
				break;
			case 'W':
				if (sscanf(trimmed_line, "W %d %u", &uid, &mid) != 2) {
					fprintf(stderr, "Event W parsing error\n");
					break;
				}
				watch_movie(uid, mid);
				break;
			case 'S':
				if (sscanf(trimmed_line, "S %d", &uid) != 1) {
					fprintf(stderr, "Event S parsing error\n");
					break;
				}
				suggest_movies(uid);
				break;
			case 'F':
				if (sscanf(trimmed_line, "F %d %d %d %u", &uid, &category1,
							&category2, &year) != 4) {
					fprintf(stderr, "Event F parsing error\n");
					break;
				}
				filtered_movie_search(uid, category1, category2, year); 
				break;
/*			case 'T':
				if (sscanf(trimmed_line, "T %u", &mid) != 1) {
					fprintf(stderr, "Event T parsing error\n");
					break;
				}
				take_off_movie(mid);
				break;
			case 'M':
				print_movies();
				break;
			case 'P':
				print_users();
				break;
*/			default:
				fprintf(stderr, "WARNING: Unrecognized event %c. Continuing...\n",
						event);
				break;
		}
	}
	fclose(event_file);
/*	destroy_structures();*/
	return 0;
}


int register_user(int uid){
	struct user *newUser = (struct user *)malloc(sizeof(struct user));
	if (newUser == NULL){
		printf("Error : Cannot allocate memory for new user");
		return -1;
	}
	/*check for already existing user id*/
	struct user *current = UsersList;

	while (current != NULL) {
        if (current->uid == uid) {
            printf("User UID %d already exists.\n", uid);
            return -1;
        }
        current = current->next;
    }
	
	newUser->suggestedHead = NULL;
	newUser->suggestedTail = NULL;
	newUser->watchHistory = NULL;
	newUser->next = NULL;
	newUser->uid = uid;
	if(UsersList == NULL){
		UsersList = newUser;
	}else{
		newUser->next = UsersList;
		UsersList = newUser;
	}
	
	printf("R <%d>\n  Users = " ,uid );
	
	current = UsersList;
	while (current != NULL) {
        printf("<uid_%d>,",current->uid);
        current = current->next;
    }
	printf("\n");

}

void unregister_user(int uid){
	struct user *current = UsersList;
	struct user *prev = NULL;

	/*Find user with the uid*/
	while(current!=NULL && current->uid != uid){
		prev = current;
		current = current->next;
	}

	if(current == NULL){
		printf("User %d did not found\n",uid);
		return;
	}
	/*FREE MEMORY of the user's suggested movie list and watch history*/
	struct suggested_movie *suggestedCurrent = current->suggestedHead;
	while(suggestedCurrent != NULL){
		struct suggested_movie *temp = suggestedCurrent;
		suggestedCurrent = suggestedCurrent->next;
		free(temp);
	}

	struct movie *watchHistoryCurrent = current->watchHistory;
	while(watchHistoryCurrent != NULL){
		struct movie *temp = watchHistoryCurrent;
		watchHistoryCurrent = watchHistoryCurrent->next;
		free(temp);
	}
		
	if(prev != NULL){
		prev->next = current->next; /*Removes user from list*/
	}else{
		UsersList = current->next; /*If user is first in the userList */
	}

	free(current);

	/*SUCCESFUL UNREGISTER*/
	printf("U <%d>\n  Users = ",uid);
	current = UsersList;
	
	while(current != NULL){
		printf("<uid_%d>",current->uid);
		current = current->next;
	}
	printf("\nDONE\n");
	
}

int add_new_movie(unsigned mid, movieCategory_t category, unsigned year){
	struct new_movie *newNewMovie = (struct new_movie*)malloc(sizeof(struct new_movie));
	if(newNewMovie == NULL){
		printf("Error : Cannot allocate memory for new movie");
		return -1;
	}
	/*Set up new movie with values from parameters*/
	newNewMovie->info.mid = mid;
	newNewMovie->info.year = year;
	newNewMovie->category = category;
	newNewMovie->next = NULL;

	struct new_movie *current = NewMoviesList;
	struct new_movie *prev = NULL;

	while(current != NULL && current->info.mid < mid){
		prev = current;
		current = current->next;
	}
	
	if(prev == NULL){
		newNewMovie->next = NewMoviesList; /*Giati NewMoviesList einai arxikopoihmeno NULL*/
		NewMoviesList = newNewMovie;
	}else{
		prev->next = newNewMovie;
		newNewMovie->next = current;
	}

	printf("A <%u><%d><%u>\n  New movies = " ,mid , category , year);
	
	current = NewMoviesList;
	while (current != NULL) {
        printf("<mid_%u,category_%d,year_%u>,",current->info.mid , current->category, current->info.year);
        current = current->next;
    }
	printf("\nDONE\n");

	return 0 ;
}

void distribute_new_movies(void){
	if(NewMoviesList == NULL){
		printf("Error : New Movies List is empty\n");
		return;
	}
	struct new_movie *temp = NewMoviesList;

	while(temp != NULL){
        struct movie *newMovie = (struct movie*)malloc(sizeof(struct movie));
        if (newMovie == NULL) {
            printf("Error: Cannot allocate memory for new Movie\n");
            return;
        }

		newMovie->info.mid = temp->info.mid;
		newMovie->info.year = temp->info.year;
		newMovie->next = NULL;

		struct movie *current;
		struct movie *prev;

		current = categoriesArray[temp->category];
		prev = NULL;

		while(current != NULL && current->info.mid < temp->info.mid){
			prev = current;
			current = current->next;
		}

		if(prev == NULL){
			newMovie->next = categoriesArray[temp->category];
			categoriesArray[temp->category] = newMovie;
		}else{
			prev->next = newMovie;
			newMovie->next = current;
		}
				
		NewMoviesList = temp->next;
		free(temp);
		temp = NewMoviesList;
	}

	printf("D\n");
	printf("Categorized Movies:\n");
	const char *categoryName;
	int i ;
	for(i = 0; i<6; i++){
		
		printf("%s:",categoryNames[i]);
		struct movie *current = categoriesArray[i];
		while (current != NULL) {
        	printf("<mid_%u,%d>,",current->info.mid ,i+1);
        	current = current->next;
    	}
		printf("\nDONE\n");
	}
}



int watch_movie(int uid, unsigned mid){
	struct movie *currentMovie;
	struct movie *prevMovie = NULL;

	struct movie *newMovie = (struct movie*)malloc(sizeof(struct movie));
	if(newMovie == NULL) {
    	return -1;
    }
	int i;
	for(i = 0; i <6; i++){
		currentMovie = categoriesArray[i];
		prevMovie = NULL;
		while(currentMovie!=NULL && currentMovie->info.mid != mid){
			prevMovie = currentMovie;
			currentMovie = currentMovie->next;
		}
		if(currentMovie == NULL) continue;
		else if(currentMovie->info.mid == mid){
			newMovie->info.mid = currentMovie->info.mid;
			newMovie->info.year = currentMovie->info.year;
			newMovie->next = NULL;
			break;
		}else{
			return-1;
		}
	}


	struct user *current = UsersList;
	struct user *prev = NULL;

	int existing_uid = 0;
	/*Find user with the uid*/
	while(current!=NULL && current->uid != uid){
		prev = current;
		current = current->next;
	}

	if(current == NULL){
		free(newMovie);
		return -1;
	}else if(current->uid == uid){
		if(current->watchHistory == NULL){
			current->watchHistory = newMovie;
		}else{
			newMovie->next = current->watchHistory;
			current->watchHistory = newMovie;
		}
	}else{
		free(newMovie);
		return -1;
	}

	printf("W <%d><%d>\n", uid , mid);
	printf("  User<%d> Watch History = ",uid);
	currentMovie = current->watchHistory;
	while(currentMovie != NULL){
		printf("<mid_%u>,",currentMovie->info.mid);
		currentMovie = currentMovie->next;
	}
	printf("\nDONE\n");
}


int suggest_movies(int uid){
	struct user *current = UsersList;

	/*Find user with the uid*/
	while(current!=NULL && current->uid != uid){
		current = current->next;
	}
	if(current == NULL){
		return -1;
	}
	
	struct user *tempUser = UsersList;
	struct movie *movieFromWatchHistory;
	int i = 0;
	
	while(tempUser != NULL){
		if(tempUser->uid == uid || tempUser->watchHistory == NULL){
			tempUser = tempUser->next;
			continue;
		}

		movieFromWatchHistory = tempUser->watchHistory;
			
		struct suggested_movie *newSuggestedMovie = (struct suggested_movie*)malloc(sizeof(struct suggested_movie));		
		if(newSuggestedMovie == NULL){
			return -1;
		}else{
	 		newSuggestedMovie->info.mid = movieFromWatchHistory->info.mid;
			newSuggestedMovie->info.year = movieFromWatchHistory->info.year;
			newSuggestedMovie->prev = NULL;
			newSuggestedMovie->next = NULL;
		}

		if((2*i)%2 == 0){
			if(current->suggestedHead == NULL){
				current->suggestedHead = newSuggestedMovie;
			}else{
				newSuggestedMovie->next = current->suggestedHead;
				current->suggestedHead->prev = newSuggestedMovie;
				current->suggestedHead = newSuggestedMovie;
			}
		}else{
			if(current->suggestedTail == NULL){
				current->suggestedTail = newSuggestedMovie;
			}else{
				newSuggestedMovie->next = current->suggestedTail;
				current->suggestedTail->prev = newSuggestedMovie;
				current->suggestedTail = newSuggestedMovie;
			}
		}
		tempUser->watchHistory = tempUser->watchHistory->next;
		tempUser = tempUser->next;
		i++;
	}

	/*Find end of suggestedHead*/
	struct suggested_movie *currHead = current->suggestedHead;
	while(currHead != NULL && currHead->next != NULL){
    	currHead = currHead->next;
	}
	/*Find end of suggestedTail*/
	struct suggested_movie *currTail = current->suggestedTail;
	while(currTail != NULL && currTail->next != NULL){
    	currTail = currTail->next;
	}

	if (currHead != NULL) {
    	currHead->next = currTail;
	}

	printf("S<%d>\n",uid);
	printf("  User<%d> Suggested Movies = ");
	
	struct suggested_movie *temp = current->suggestedHead;
    while (temp != NULL) {
        printf("<mid_%u>,", temp->info.mid);
        temp = temp->next;
    }
    printf("\nDONE\n");
}


int filtered_movie_search(int uid, movieCategory_t category1,movieCategory_t category2, unsigned year){
	struct suggested_movie *suggestedMoviesList = NULL;

	struct movie *currentMovie = categoriesArray[category1];
	while(currentMovie != NULL){
		
		if(currentMovie->info.year >= year){
			struct suggested_movie *tempSuggestedMovie = (struct suggested_movie*)malloc(sizeof(struct suggested_movie));
			if(tempSuggestedMovie == NULL){
				return -1;
			}
			tempSuggestedMovie->info.mid = currentMovie->info.mid;
			tempSuggestedMovie->info.year = currentMovie->info.year;
			tempSuggestedMovie->next = NULL;
			tempSuggestedMovie->prev = NULL;
			
			/* Insert in ascending order based on mid*/
            if(suggestedMoviesList == NULL || suggestedMoviesList->info.mid > currentMovie->info.mid) {
                tempSuggestedMovie->next = suggestedMoviesList;
                if(suggestedMoviesList != NULL){
                    suggestedMoviesList->prev = tempSuggestedMovie;
                }
                suggestedMoviesList = tempSuggestedMovie;
            }else{
                struct suggested_movie *currentSuggestedMovie = suggestedMoviesList;
                while(currentSuggestedMovie->next != NULL && currentSuggestedMovie->next->info.mid < currentMovie->info.mid) {
                    currentSuggestedMovie = currentSuggestedMovie->next;
                }

                tempSuggestedMovie->next = currentSuggestedMovie->next;
                if(currentSuggestedMovie->next != NULL){
                    currentSuggestedMovie->next->prev = tempSuggestedMovie;
                }
                tempSuggestedMovie->prev = currentSuggestedMovie;
                currentSuggestedMovie->next = tempSuggestedMovie;
            }

			currentMovie = currentMovie->next;
		}

	}

	currentMovie = categoriesArray[category2];
while (currentMovie != NULL) {
    if (currentMovie->info.year >= year) {
        /* Insert logic here (similar to category1) */
        struct suggested_movie *tempSuggestedMovie = (struct suggested_movie*)malloc(sizeof(struct suggested_movie));
        if(tempSuggestedMovie == NULL){
            return -1;
        }
        tempSuggestedMovie->info.mid = currentMovie->info.mid;
        tempSuggestedMovie->info.year = currentMovie->info.year;
        tempSuggestedMovie->next = NULL;
        tempSuggestedMovie->prev = NULL;
        
        /* Insert in ascending order based on mid*/
        if (suggestedMoviesList == NULL || suggestedMoviesList->info.mid > currentMovie->info.mid) {
            tempSuggestedMovie->next = suggestedMoviesList;
            if(suggestedMoviesList != NULL){
                suggestedMoviesList->prev = tempSuggestedMovie;
            }
            suggestedMoviesList = tempSuggestedMovie;
        } else {
            struct suggested_movie *currentSuggestedMovie = suggestedMoviesList;
            while(currentSuggestedMovie->next != NULL && currentSuggestedMovie->next->info.mid < currentMovie->info.mid) {
                currentSuggestedMovie = currentSuggestedMovie->next;
            }

            tempSuggestedMovie->next = currentSuggestedMovie->next;
            if(currentSuggestedMovie->next != NULL){
                currentSuggestedMovie->next->prev = tempSuggestedMovie;
            }
            tempSuggestedMovie->prev = currentSuggestedMovie;
            currentSuggestedMovie->next = tempSuggestedMovie;
        }
    }

    currentMovie = currentMovie->next;
}

printf("F<%d><category_%d><category_%d><year_%u>\n",uid,category1,category2,year);

struct user *current = UsersList;

/* Find user with the uid */
while(current!=NULL && current->uid != uid){
    current = current->next;
}
if(current == NULL){
    return -1;
}

/* Fix for traversing the suggestedMoviesList to find the end */
struct suggested_movie *temp = suggestedMoviesList;
while (temp != NULL && temp->next != NULL) {
    temp = temp->next;
}

if (temp != NULL) {
    temp->next = current->suggestedHead;
}
else {
    suggestedMoviesList = current->suggestedHead;
}

/* Print the suggested movie list */
temp = suggestedMoviesList;
printf("F<%d><category_%d><category_%d><year_%u>\n",uid,category1,category2,year);
printf("  User<%d>Suggested Movies = ",uid);
while(temp != NULL){
    printf("<mid_%u>,",temp->info.mid);
    temp = temp->next;
}
printf("\nDONE\n");

return 0;
}

