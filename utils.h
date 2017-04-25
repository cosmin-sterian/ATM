typedef struct clients {
	char nume[13];
	char prenume[13];
	int nr_card;
	int pin;
	char parola[17];
	double sold;
	int logged_in;
	int attempts_left;
	int socket;
} clients;

int checkInput(char *string) {
	//0 - Eroare | 1 - login | 2 - logout | 3 - listsold | 4 - getmoney | 5 - putmoney | 6 - unlock | 7 - quit |
	if(strstr(string, "login") != NULL)
		return 1;	//login
	if(strstr(string, "logout") != NULL)
		return 2;	//logout
	if(strstr(string, "listsold") != NULL)
		return 3;	//listsold
	if(strstr(string, "getmoney") != NULL)
		return 4;	//getmoney
	if(strstr(string, "putmoney") != NULL)
		return 5;	//putmoney
	if(strstr(string, "unlock") != NULL)
		return 6;	//unlock
	if(strstr(string, "quit") != NULL)
		return 7;	//quit

	return 0;	//eroare
}
