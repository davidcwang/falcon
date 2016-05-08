#ifndef MOUSE_CONTROL_H
#define MOUSE_CONTROL_H


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


#define NORTH_WEST_CELL 0
#define NORTH_EAST_CELL 1
#define SOUTH_WEST_CELL 2
#define SOUTH_EAST_CELL 3




int count = 2;

void get_next_move();

//Called when position update is called by drive
void update_position() {
    UPDATING_INSTRUCTIONS = true;
    UPDATE_POSITION = false;
    UPDATE_FINISHED = false;
    
    ledMf = 1;
    
    
    
    int y = mouse_y;
    int x = mouse_x;



    set_wall(y,x);

    if (maze[y][x]->top_wall || maze[y][x]->right_wall) {
        update_distances(stack);
    }
    Cell *curr = next_move(maze[y][x]);
    
    get_next_move();
    
    mouse_y = curr->y;
    mouse_x = curr->x;
    
    
    //
//    //Update to next cell first
//    if (count < 3){
//        mouse_action = DRIVE;    
//    }
//    else if (count < 4){
//        mouse_action = TURN_RIGHT;
//    }
//    else if (count < 5) {
//        mouse_action = TURN_RIGHT;
//    }
//    else if (count < 6) {
//        mouse_action = DRIVE;
//    }
//    else if (count < 7){
//        mouse_action = TURN_LEFT;
//    }
//    else if (count < 19){
//        mouse_action = DRIVE;
//    }
//    else if (count < 20){
//        mouse_action = TURN_LEFT;
//    }
//    else if (count < 29){
//        mouse_action = DRIVE;
//    }
//    else if (count < 30){
//        mouse_action = TURN_LEFT;
//    }
//    else if (count < 31){
//        mouse_action = TURN_LEFT;
//    }
//    else if (count < 32){
//        mouse_action = DRIVE;
//    }
//    else if (count < 33) {
//        mouse_action = TURN_RIGHT;
//    }
//    else if (count < 34){
//        mouse_action = TURN_LEFT;
//    }
    //else {
//        drive_ticker.detach();
//        stop();
//        wait(10000);
//    }
//    
//    pc.printf("count: %d\r\n", count);
//    //wait(0.1);
//    count++;
//    
//    //Set walls
//    //then call update_maze();
    
    ledMf = 0;
    UPDATING_INSTRUCTIONS = false;
    //UPDATE_FINISHED = true;
}

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
    update_position();
    
    UPDATE_POSITION = false;
    UPDATING_INSTRUCTIONS = false;
    DONE_MOVING = true;
    ledYellow = 0;
}


void save_valid_maze();


void arrived_at_beginning() {
    global_state = WAITING;
    
    while (!DONE_MOVING) {
        //Wait for move to finish
    }
    
    //Verify that accurately arrived at beginning. If not, lost
    if (current_direction == SOUTH && leftIR.readIR() < 12 && rightIR.readIR() < 12 && rightFrontIR.readIR() < 12){
        save_valid_maze();
        global_state = AT_BEGINNING;
    }
    else {
        /*global_state = LOST*/;
        
    }
}
//Drives a circle at center and 
bool verify_at_center(){
    
    //bool is_left = true;
//    switch(which_center_cell){
//        case NORTH_WEST_CELL: 
//            switch(current_direction){
//                case EAST: is_left = true;
//                case SOUTH: is_left = false; 
//            }
//            break;
//        case NORTH_EAST_CELL: 
//            switch(current_direction){
//                case WEST: is_left = false;
//                case SOUTH: is_left = true; 
//            }
//            break;
//        case SOUTH_WEST_CELL: 
//            switch(current_direction){
//                case EAST: is_left = false;
//                case NORTH: is_left = true; 
//            }
//            break;
//        case SOUTH_EAST_CELL: 
//            switch(current_direction){
//                case WEST: is_left = true;
//                case NORTH: is_left = false; 
//            }
//            break;
//        
//    }
//    
//    //drive will be updating this
//    switch(current_direction){
//        case NORTH: next_direction = SOUTH; break;
//        case SOUTH: next_direction = NORTH; break;
//        case EAST: next_direction = WEST; break;
//        case WEST: next_direction = EAST; break;
//    }
//    
//    if (is_left){
//        drive_cell();
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_left(&drive_cell);
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_left(&drive_cell);
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_left(&drive_cell);
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_right();
//    }
//    else {
//        drive_cell();
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_right(&drive_cell);
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_right(&drive_cell);
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_right(&drive_cell);
//        while(!DONE_MOVING) { if (user_button) { return false;} if (global_state == LOST) {return false;} }
//        turn_left();
//    }
    
    return true;
}

void arrived_at_center() {
    global_state = WAITING;
    
    //Wait for arrival
    while(!DONE_MOVING) { if (user_button) { /*global_state = LOST*/; } }
    
    //Drive all 4 center cells to verify that is at center
    if (verify_at_center()){
        save_valid_maze();                   
        global_state = AT_CENTER;
    }
    else {
        /*global_state = LOST*/;
    }

}

//Called by maze after computing
//Uses current direction and direction of next cell to compute the next move
void get_next_move(){
    if (is_center(maze[mouse_y][mouse_x])){
        next_direction = IS_AT_CENTER;
    }
    
    
    switch(next_direction){
        case IS_AT_BEGINNING: 
            arrived_at_beginning(); //Blocking this main thread and set status to WAITING in meantime. Drive can finish
        break;
        case IS_AT_CENTER: 
            arrived_at_center(); //Blocking this main thread and set status to WAITING in meantime. Drive can finish.
        break;
        case NORTH: 
            switch (current_direction) {
                case NORTH: mouse_action = DRIVE;
                case WEST: mouse_action = TURN_RIGHT;
                case SOUTH: mouse_action = TURN_AROUND;
                case EAST: mouse_action = TURN_LEFT;
            }
            break;
        case WEST: 
            switch (current_direction) {
                case NORTH: mouse_action = TURN_LEFT;
                case WEST: mouse_action = DRIVE;
                case SOUTH: mouse_action = TURN_RIGHT;
                case EAST: mouse_action = TURN_AROUND;
            }
            break;
        case SOUTH: 
            switch (current_direction) {
                case NORTH: mouse_action = TURN_AROUND;
                case WEST: mouse_action = TURN_LEFT;
                case SOUTH: mouse_action = DRIVE;
                case EAST: mouse_action = TURN_RIGHT;
            }
            break;
        case EAST: 
            switch (current_direction) {
                case NORTH: mouse_action = TURN_RIGHT;
                case WEST: mouse_action = TURN_AROUND;
                case SOUTH: mouse_action = TURN_LEFT;
                case EAST: mouse_action = DRIVE;
            }
            break;
    }
    
    ledRed = 0;
    ledYellow = 0;
    ledGreen = 0;
    ledMf = 0;

    switch(mouse_action){
        case DRIVE: ledRed = 0;
    ledYellow = 0;
    ledGreen = 0;
    ledMf = 0;
    ledYellow = 1; break;
        case TURN_RIGHT: ledGreen = 1; break;
        case TURN_LEFT: ledRed = 1; break;
        case TURN_AROUND: ledMf = 1; break;
    }
}




//Updates ALL distance values in maze, so will be accurate on next run
void full_update_maze(){

}

void save_valid_maze(){
    full_update_maze();
    
    
    //then saves    
}

//Restores assuming at beginnning
void restore_valid_maze(){
    //current_cell = beginning_cell;
}




void lost(){
    while(user_button){
        //Wait for button let go, if any
    }
    //ledRed = 1;
//    ledGreen = 1;
//    ledYellow = 1;
//    
//    drive_ticker.detach();
//    stop();
//    restore_valid_maze();
//    
//    while(!user_button){
//        //Wait for button press
//    }
//    while(user_button){
//        //Wait for button let go
//    }
    
    global_state = AT_BEGINNING;
}




bool not_fetched = true;
int last_encoder_val = 0;
void at_beginning(){
    int encoder_val = leftEncoder;
    
    //if (mouse_mode == SPEED_RUN){
//        ledYellow = 1;
//        ledGreen = 0;
//    }
//    else {
//        ledYellow = 0;
//        ledGreen = 1;
//    }
//    
//    if (battery.read() < 0.75f){
//        ledRed = 1;
//    }
//    
//    //On press, takes off
//    if (user_button){
//        
//        resetEncoders();
//        drive_ticker.detach();
//        if (mouse_mode == SPEED_RUN){
//            ledYellow = 1;
//            ledGreen = 0;
//        }
//        else {
//            ledYellow = 0;
//            ledGreen = 1;
//        }
//        while(user_button){
//            //Wait for release
//        }
//        
//        if (mouse_mode == SPEED_RUN){
//            //Set speed
//            resetEncoders();
//            while(!user_button){
//                if (leftEncoder < 0){
//                    resetEncoders();
//                }
//                else if (leftEncoder < 1000){
//                    buzzer.play_async(_C0);                            
//                    drive_top_speed = 0.1;
//                    turn_top_speed = 0.2;
//                }
//                else if (leftEncoder < 2000){
//                    buzzer.play_async(_D1);       
//                    drive_top_speed = 0.12;
//                    turn_top_speed = 0.24;
//                }
//                else if (leftEncoder < 3000){
//                    buzzer.play_async(_E2);       
//                    drive_top_speed = 0.14;
//                    turn_top_speed = 0.28;
//                }
//                else if (leftEncoder < 4000){
//                    buzzer.play_async(_F5);       
//                    drive_top_speed = 0.16;
//                    turn_top_speed = 0.32;
//                }
//                else if (leftEncoder < 5000){
//                    buzzer.play_async(_G6);
//                    drive_top_speed = 0.18;
//                    turn_top_speed = 0.36;
//                }
//                else if (leftEncoder < 6000){
//                    buzzer.play_async(_Ab7);
//                    drive_top_speed = 0.20;
//                    turn_top_speed = 0.40;
//                }
//                
//                wait(0.5);
//            }
//            
//            wait(2);
//            global_state = SPEEDING_TO_CENTER;
//            return;
//        }
//        else {
//            //Start explore
//            drive_top_speed = 0.1;
//            turn_top_speed = 0.2;
//            wait(2);
//            global_state = EXPLORING_TO_CENTER;
//            return;
//        }
//    }
//    
//    
//    wait(0.5);
//    if (leftEncoder - encoder_val > 1000 || leftEncoder - encoder_val < -1000 ){
//        if (mouse_mode == SPEED_RUN){
//            mouse_mode = EXPLORE;
//        }
//        else {
//            mouse_mode = SPEED_RUN;
//        }
//    }
}

void at_center(){
    //Set maze to be ready to drive towards beginning
    
    global_state = EXPLORING_BACK;
}

void exploring_to_center() {
    if (UPDATE_POSITION) {
        update_position();
    }
    
    if (user_button){
        /*global_state = LOST*/;
    }
    
    if (UPDATING_INSTRUCTIONS) {
        //Wait for instructions to finish
    }
    else if (DONE_MOVING) {
        UPDATE_FINISHED = false;
        switch (mouse_action) {
            case DRIVE:
                drive_cell();
                break;
            case TURN_RIGHT:
                turn_right(&drive_cell);
                
                break;
            case TURN_LEFT:
                turn_left(&drive_cell);
                
                break;
            case TURN_AROUND:
                turn_around(&drive_cell);
                
                break;
        }
    }
}

void exploring_back() {
    if (UPDATE_POSITION) {
        update_position();
    }
    
    
    if (user_button){
        /*global_state = LOST*/;
    }
    
    if (UPDATING_INSTRUCTIONS) {
        //Wait for instructions to finish
    }
    else if (DONE_MOVING) {
        UPDATE_FINISHED = false;
        switch (mouse_action) {
            case DRIVE:
                drive_cell();
                break;
            case TURN_RIGHT:
                turn_right(&drive_cell);
                break;
            case TURN_LEFT:
                turn_left(&drive_cell);
                break;
            case TURN_AROUND:
                turn_around(&drive_cell);
                break;
        }
    }
}


void speed_drive_with_distance(){
    speed_drive_cell(drive_distance);
}

void speeding_to_center() {
    if (UPDATE_POSITION) {
        update_position();
    }
    
    if (user_button){
        /*global_state = LOST*/;
    }

    if (UPDATING_INSTRUCTIONS) {
        //Wait for instructions to finish
    }
    else if (DONE_MOVING) {
        UPDATE_FINISHED = false;
        switch (mouse_action) {
            case DRIVE:
                speed_drive_cell(drive_distance);
                break;
            case TURN_RIGHT:
                speed_turn_right(speed_drive_with_distance);
                
                break;
            case TURN_LEFT:
                speed_turn_left(speed_drive_with_distance);
                
                break;
            case TURN_RIGHT_DIAGONAL:
                speed_turn_right_diagonal(speed_drive_with_distance);
                
                break;
            case TURN_LEFT_DIAGONAL:
                speed_turn_left_diagonal(speed_drive_with_distance);
                
                break;
            case DRIVE_DIAGONAL:
                break;//TODO
        }
    }
}

void speeding_back() {
    if (UPDATE_POSITION) {
        update_position();
    }
    
    if (user_button){
        /*global_state = LOST*/;
    }
    
    if (UPDATING_INSTRUCTIONS) {
        //Wait for instructions to finish
    }
    else if (DONE_MOVING) {
        UPDATE_FINISHED = false;
        switch (mouse_action) {
            case DRIVE:
                speed_drive_cell(drive_distance);
                break;
            case TURN_RIGHT:
                speed_turn_right(speed_drive_with_distance);
                
                break;
            case TURN_LEFT:
                speed_turn_left(speed_drive_with_distance);
                
                break;
            case TURN_RIGHT_DIAGONAL:
                speed_turn_right_diagonal(speed_drive_with_distance);
                
                break;
            case TURN_LEFT_DIAGONAL:
                speed_turn_left_diagonal(speed_drive_with_distance);
                
                break;
            case DRIVE_DIAGONAL:
                break;//TODO
        }
    }
}













#endif