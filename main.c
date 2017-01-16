#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#ifdef RASPBERRY_PI
#include <wiringPi.h>
#endif //RASPBERRY_PI

#define NUM_MEMBER 30
#define LENGTH_NAME_PLUS_NUMBER 80
#define PIN_ACTIVE 0	//Read 0 or one if pin is active?
#define INPUT_PIN 5	//Choose your input pin
#define DEBOUNCE_TIME 100000 //Debounce input pin with 100ms given in us

char *prog_name=NULL;
FILE *logfile=NULL;
FILE *file_tel_numbers=NULL;
char *tel_numbers=NULL;

void usage(void) {
	fprintf(stderr,"%s: %s <telephone numbers>",prog_name,prog_name);
}

void close_all() {
	free(tel_numbers);
	fclose(logfile);
	fclose(file_tel_numbers);
	return;
}

void bailout(char *msg) {
	fprintf(stderr,"%s: %s\n",prog_name,msg);
	fprintf(logfile,"%s\n",msg);
	fprintf(logfile,"---------------------Stop alarm---------------------------\n");
	close_all();
	exit(EXIT_FAILURE);
}

static void sig_handler(int id) {
	bailout("Manual stop");
}

int main(int argc, char *argv[]) {

	int i;
	int num_members;
	size_t buffer_size;
	char *first_blank;
	prog_name = argv[0];
	char gammu_cmd[100];

	//Log file
	logfile=fopen("/home/pi/SmsAlarm/log.txt","a");
	if (logfile == NULL ) {
		fprintf(stderr,"%s: Coudn't open log file",prog_name);
	}
	
	fprintf(logfile,"---------------------Start alarm--------------------------\n");

	//Signal handler
	if (signal(SIGINT, sig_handler) == SIG_ERR){
		bailout("Coudn't install signal handler");
	}

	if(argc != 2) {
		usage();
		bailout("Wrong number of arguments!!!");
	}

	//Read in the telephone numbers
	file_tel_numbers = fopen(argv[1],"r");
	if(file_tel_numbers == NULL) {
		bailout("Couldn't open file with telephone numbers");
	}

	buffer_size = (size_t)NUM_MEMBER*sizeof(char)*LENGTH_NAME_PLUS_NUMBER;
	tel_numbers = (char *) malloc(buffer_size);
	if(tel_numbers == NULL){
		bailout("Faild to allocate memory");
	}

#ifdef RASPBERRY_PI
	if (wiringPiSetup() == -1) {
    		bailout("Faild to setup WiringPi API");
	}
	
	//TODO: Error Handling
	pinMode(INPUT_PIN, INPUT);

	//TODO: Error Handling
	pullUpDnControl(INPUT_PIN, PUD_UP);
#endif //RASPBERRY_PI

	//Read telnumber and name of max NUM_MEMBERS members, but max LENGTH_NAME_PLUS_NUMBER characters
	for (i=0;i<NUM_MEMBER;i++) {
		if(!fgets(tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER),LENGTH_NAME_PLUS_NUMBER,file_tel_numbers)||ferror(file_tel_numbers)||feof(file_tel_numbers)){
			break;
		}
	}

	//Store the number of members
	num_members = i;

	//delete names
	for(i=0;i<num_members;i++){
		fprintf(logfile,"%d.) %s\n",i,tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER));
		//TODO: uncomment, only for debug
		printf("%d.) %s\n",i,tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER));
		first_blank = strchr(tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER),' ');
		*first_blank = '\0';
	}

	fflush(logfile);

	while(1) {
#ifdef RASPBERRY_PI
		if(digitalRead(INPUT_PIN)==PIN_ACTIVE){
#else
		sleep(3);
		if(1){
#endif //RASPBERRY_PI
			//TODO: Remove for debug only
			fprintf(stdout,"debounce\n");
			fflush(stdout);
			usleep(DEBOUNCE_TIME);
#ifdef RASPBERRY_PI
			if(digitalRead(INPUT_PIN)==PIN_ACTIVE){
#else
			if(1){
#endif //RASPBERR_PI
				//TODO: Remove for debug only
				fprintf(stdout,"Alarm\n");
				fflush(stdout);
				fprintf(logfile,"Alarm\n");
				
				//Send alarm sms to all telephone numbers
				for (i=0; i<num_members; i++) {
					fprintf(logfile,"Send SMS to Member %d with number %s\n",i,tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER));
					//TODO: remove for debug only
					printf("Send SMS to Member %d with number %s\n",i,tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER));
					sprintf(gammu_cmd,"gammu sendsms TEXT %s -text \"Feuerwehr Alarm\"",tel_numbers+(i*LENGTH_NAME_PLUS_NUMBER));
					fprintf(logfile,"Cmd: %s\n",gammu_cmd);
					//TODO: Errohandling, check if sms was realy sent
					system(gammu_cmd);
					fflush(logfile);
					
				}
				fflush(logfile);			
			}
		}
	}
	return 0;
}
