#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define NORTH_WEST 4
#define NORTH_EAST 5
#define SOUTH_WEST 6
#define SOUTH_EAST 7
#define IS_AT_BEGINNING 8
#define IS_AT_CENTER 9


//Internal for elsewhere
int next_cell_direction = 0; //TOP
int direction; //maze internal

//Signals from/to elsewhere:
int current_direction = NORTH;
int next_direction = -1; //A direction or AT_BEGINNING or AT_CENTER
int drive_distance = 1; //MAZE MUST SPECIFY DISTANCE TO DRIVE IN SPEED DRIVE MODE

int mouse_x = 0;
int mouse_y = 0;



float drive_top_speed = 0.1;
float turn_top_speed = 0.2;

bool has_left_wall = true;
bool has_right_wall = true;

bool UPDATE_POSITION = false;     //Needs position update when drive into new cell
bool DONE_MOVING = true;              //Driving or turning complete
bool UPDATING_INSTRUCTIONS = true;    //Control alg not yet saved an action to mouse_action
bool UPDATE_FINISHED = false;       //Contorl alg has saved next action to mouse_action. Will always be false unless just finshed updating and next action has not been used yet
bool break_loop = false;

bool LAST_ACTION_WAS_DRIVE = false;



enum MouseMode {
    EXPLORE,
    SPEED_RUN
};
MouseMode mouse_mode = EXPLORE;

enum GlobalState {
    STARTUP,
    WAITING,
    AT_BEGINNING,
    EXPLORING_TO_CENTER,
    EXPLORING_BACK,
    AT_CENTER,
    SPEEDING_TO_CENTER,
    SPEEDING_BACK,
    LOST
};
GlobalState global_state;

enum MouseAction {
    DRIVE,
    TURN_RIGHT,
    TURN_LEFT,
    TURN_AROUND,
    TURN_LEFT_DIAGONAL,
    TURN_RIGHT_DIAGONAL,
    DRIVE_DIAGONAL,
    ARRIVED_AT_CENTER,
    ARRIVED_AT_BEGINNING
};
MouseAction mouse_action;


#include "maze.h"
vector<Cell*> stack;
#include "drive_control.h"
#include "io.h"


Serial pc(PA_9, PA_10);  //THIS MUST GO IN MAIN
//Serial pc(D1, D0);

void move_one_forward(){

    init_pid_consts();
        resetEncoders();
    while(!break_loop)
    {
        drive_cell();
    }
    break_loop = false;
    resetEncoders();
    init_pid_consts();
    
    return;   

}
/*
void turn_one_right(){
    init_pid_consts();
    while(!break_loop)
    {
        turn_right();
    }
    break_loop = false;
    resetEncoders();
    init_pid_consts();
}*/
void setup() {
    global_state = STARTUP;
    pc.baud(9600);
    
    print_battery();
    if (battery.read() < 0.73f){
        //ledRed = 1;
    }
    
    
    init_maze();
    
    global_state = EXPLORING_TO_CENTER;
    
    mouse_action = DRIVE;
    
    mouse_x = 0;
    mouse_y = 0;
    has_right_wall =  true;
    has_left_wall = true;

    
    UPDATE_POSITION = false;
    UPDATING_INSTRUCTIONS = false;
    DONE_MOVING = true;
    ledYellow = 0;
}
int main() {
    setup();

    while(!user_button){

    }
    wait(2); 
    _drive_init();
 //   while (1) {
    //    drive_cell();
      //  while(!UPDATE_POSITION){
            /*
            for(;;){
            //    drive_cell();
                if(!has_right_wall && has_left_wall){
                    wait(3);
                    ledGreen = 1;
                    ledRed = 0;
                    ledYellow = 0;
                    turn_right(drive_cell);
                    
                }
                else if(!has_left_wall && has_right_wall){
                    wait(3);
                    turn_left(drive_cell);
                    ledGreen = 0;
                    ledRed = 1;
                    ledYellow = 0;
                                
                }
                else{
                    wait(3);
                    drive_cell();
                    ledGreen = 0;
                    ledRed = 0;
                    ledYellow = 1;
                    
                }
            }*/  
                move_one_forward();
                wait(0.5);
                
                turn_right(drive_cell);


                
                
                
                ledRed = 0;
           //stop();
            
                    



   // }
}