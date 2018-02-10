//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include<fstream>
#include<iostream>
#include<bits/stdc++.h>
#include<iostream>
#include<unistd.h>
#include<ncurses.h>
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
using namespace std ; 
#define TRUE   1 
#define FALSE  0 

//Used classes 
class SocketHandler ; 
class Game ; 
class snake ; 
class snake_part ; 



//global variables for the program 
int max_x  = 0 , max_y = 0  ;  //Make max_x and max_y as global since the values are used by many methods 
ofstream logfile;


typedef struct food
{
    int x,y ;
    char foodChar ;
}food ; 


class SocketHandler{

    struct sockaddr_in address;
    struct sockaddr_in serv_addr;
    int sock ;  
    char buffer[1024]  ; 
    fd_set set;
    struct timeval timeout;

    public : 
    SocketHandler(void) ; 
    void sendData(string) ; 
    void createClientSocket(void) ; 
    void initServerAddress(string , int) ; 
    int connectToServer(string , int) ; 
    void readData() ; 
    void closeSocket() ; 
};


class Game 
{
    private:
    int noOfPlayers ;
    food foodObj ;
    int center_x , center_y; 
    unsigned long int speed ; 

    public:

    Game(int players = 1 )
    {
        noOfPlayers= players ; 
        speed = 70000; 
        foodObj.x  = foodObj.y = 0 ;
    }
    
    SocketHandler sock_obj; 
    void generateFood() ; 
    void printFood(string ) ;
    void setFoodPos(int , int) ;
    void handleMessageFromServer(string ) ; 
    int getFoodX()
    {return foodObj.x ; }
    int getFoodY()
    {return foodObj.y; }
    int getCenterX(){return center_x ; }
    int getCenterY(){return center_y ; }
    food getFoodPos() ;
    int getNoOfPlayers(){return noOfPlayers ; }
    void reset_max_screen() ; 
    void setNoOfPlayers(int n){noOfPlayers  = n ; }
    void initConsoleScreen(string ) ; 
    unsigned long int setSpeed(unsigned long int s){speed = s>200000?200000:s<10000?10000:s ; return speed ; } //Speed Increments by 3000
    long int getSpeed(void){return speed; }
};
Game GameObj(1) ;

//Setter method to set the position of food 
void Game::setFoodPos(int x , int y)
{
   foodObj.x = x ; 
   foodObj.y = y ;  
}

void Game::handleMessageFromServer(string msg)
{
    // cout<<"\n\nServer sent :"<<msg ; 
    // cout.flush() ; 
    // sleep(3) ; 
    logfile<<"\nServer sent msg:" <<msg <<"\n" ; 

    if(msg.find(":")!=string::npos)
    {
        int start_colon = msg.find(":") ; 
        
        int camma = msg.find(",") ; 
        string str_x = msg.substr(start_colon+1 , camma-1) ;
        string str_y = msg.substr(camma+1 , 3) ; 

        // GameObj.initConsoleScreen("off") ; 
        int x  = stoi(str_x)  , y = stoi(str_y) ; 
        // cout<<"x = "<<x <<" y= "<<y ; 
        // cout.flush() ;

        // GameObj.initConsoleScreen("on") ; 

        GameObj.setFoodPos(x , y) ; 

    }

    else{
        for(int i =0 ; i<msg.length()  ; i++)
        {
            if(msg[i]=='-')
                GameObj.setSpeed(GameObj.getSpeed()+3000) ;             
            else if(msg[i]=='+')
                GameObj.setSpeed(GameObj.getSpeed()-3000) ; 
        }
    }
}


//Initialise the console with the decision to turn off or on the enter key and cursor
void Game::initConsoleScreen(string state)
{
    if(state=="on")
    {
        initscr() ; //Init screen 
        noecho() ; // Dont show any pressed char  
        curs_set(false) ; // Don't show the cursor 
            

        getmaxyx(stdscr , max_y , max_x) ; 
        center_x = max_x/2  , center_y = max_y/2 ; 
        
        cbreak() ; //Dont wait for enter to be pressed when using getch 
        nodelay(stdscr , 1) ;  //Use non blocking input for getch which just returns ERR if there is no input (ERR=-1)

    }

    else if(state=="off"){
        endwin() ; 
    }
}

//Returns food obj with a copy of data of foodObj member of game class
food Game::getFoodPos()
{
    food obj = foodObj ;
    return obj ;
}


void Game::generateFood()
{
    int x = random()%max_x , y = random()%max_y  ; 
    if(!x)x = 2 ; 
    if(!y) y = 2 ; 
    mvprintw(y, x ,"#") ;   
    setFoodPos(x , y) ;
}


void Game::printFood(string status="old")
{
    if(status=="new")
        GameObj.generateFood() ; 
    
    if(!GameObj.getFoodX() && !GameObj.getFoodY())
        generateFood() ; 
    mvprintw(foodObj.y, foodObj.x ,"#") ;   
}



    //initialise all client_socket[] to 0 so not checked 
    SocketHandler::SocketHandler(void)
    {
        sock = 0  ;  
        memset(&serv_addr, '0', sizeof(serv_addr));
        memset(buffer , 0 ,sizeof(buffer)) ; 
        timeout.tv_sec = timeout.tv_usec = 0 ; 
    }

    void SocketHandler::sendData(string message)
    {
        logfile<<"\nSending data to server : \n" <<message <<"\n" ; 
        send(sock , message.c_str() , message.length() , 0 );
        // cout<<"Message sent : " << message ; 
    }

    //creates and assigns the fd value to sock member
    void SocketHandler::createClientSocket(void)
    {
        int opt= 1 ;
        sock = socket(AF_INET , SOCK_STREAM , 0) ;
        FD_ZERO(&set); /* clear the set */
        FD_SET(sock, &set); /* add our file descriptor to the set */


        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
        }

        if (setsockopt(sock , SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
    }

    void SocketHandler::initServerAddress(string address , int port )
    {
    
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        
        // Convert IPv4 and IPv6 addresses from text to binary form
        if(inet_pton(AF_INET,address.c_str(), &serv_addr.sin_addr)<=0) 
        {
            printf("\nInvalid address/ Address not supported \n");
        }
    }


    int SocketHandler::connectToServer(string address , int port)
    {
        createClientSocket() ; 
        initServerAddress(address , port) ; 

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            exit(1) ;
        }
    }



    void SocketHandler::readData()
    {
        // valread = read( sock , buffer, 1024);
        int val ; 

        FD_ZERO(&set); /* clear the set */
        FD_SET(sock, &set); /* add our file descriptor to the set */

        val = select(sock+1 , &set , NULL , NULL , &timeout) ; 

        if(val>0)
        {
            recv(sock , buffer, 1024, 0);
            // cout<<"got message : " <<buffer ; 
            
            GameObj.handleMessageFromServer(string(buffer)) ;

            //Handle the message from server and interpret the results . Call the message handler method 
        }

        else if(val==-1)
        {
            perror("select") ; 
        }
    }


    void SocketHandler::closeSocket()
    {
        close(sock) ; 
    }




class snake_part
{
    public :
    int x , y ; 

    snake_part(int  xx, int yy )
    {
        x = xx ; y = yy ; 
    }
};



class snake
{
    private :
    vector <snake_part> parts ;
    string snakeDirection  ;
    int score ;
    string player_name ; 
    char keyUp , keyDown , keyRight , keyLeft ; 
    int id ; 

    public :
    snake(char up , char down , char right , char left , int snakeid ,string name="Name" )
    {
        keyUp = up , keyDown = down , keyRight = right , keyLeft = left ; 
        score = 0 ; 
        snakeDirection = "right" ; 
        id = snakeid ; 
        player_name = name ; 
        
    }

    int getScore(void){return score ; }
    int setScore(int s){score = s ; return score ; }

    void draw_snake(void)
    {
        int i ; 
        for(  i =0 ; i<parts.size()-1 ; i++)
        {
            mvprintw(parts[i].y  , parts[i].x , "o") ; 
        }
        mvprintw(parts[i].y  , parts[i].x , "+") ; 
    }

    void add_part(int x , int y , string direction = "right" ) //adds the part object taking the coordinates to the end of the part vector in snake
    {
        snake_part obj(x , y) ; 
        if(direction=="right")
            parts.push_back(obj) ; 

        else if(direction=="left")
            parts.insert(parts.begin() ,obj) ; 
    }

    //called to make the snake appear on the screen for the first time 
    void init_snake_on_screen()
    {
        add_part(GameObj.getCenterX() , GameObj.getCenterY()) ; 
        add_part(GameObj.getCenterX()+1 , GameObj.getCenterY()) ; 
        add_part(GameObj.getCenterX()+2 , GameObj.getCenterY()) ; 
        
        draw_snake() ;
    }

    //Used to move the snake in the given direction 
    void move_snake(string direction)
    {
        
        parts.erase(parts.begin())  ;
        snake_part last_part = parts.at(parts.size()-1) ;

        if(direction=="right")
        {
            add_part((last_part.x+1)%max_x  , last_part.y) ;
            snakeDirection  = "right" ; 
        }

        else if(direction =="left") 
        {
            add_part((last_part.x-1)<0?max_x-1:(last_part.x-1) , last_part.y) ; 
            snakeDirection = "left" ; 
        }

        else if(direction =="up")
        {
            add_part(last_part.x , (last_part.y-1)<0?max_y-1:(last_part.y-1)) ;
            snakeDirection = "up" ; 
        }
        else if(direction=="down")
        {
            add_part(last_part.x , (last_part.y+1)%max_y) ;
            snakeDirection = "down" ; 
        }

        check_snake_overlap() ;

        if(getHeadX()==GameObj.getFoodX() && getHeadY() == GameObj.getFoodY())
        {
            add_part(GameObj.getFoodX() , GameObj.getFoodY() ) ;
            setScore(getScore()+1) ; 
            GameObj.sock_obj.sendData("#") ; 
            
            //clear that food now  
            GameObj.setFoodPos(-10 , -10) ; 
            // GameObjintFood("new") ;
        }

        draw_snake() ;
        refresh() ; 
    }

//Checks if the snake bites itself or not ! :D 
    int check_snake_overlap()
    {
    int headX = getHeadX() , headY  = getHeadY() ; 
    for(int i =0 ; i<parts.size()-1 ; i++)
        if(parts[i].x==headX && parts[i].y==headY)
            {

                clear() ;
                mvprintw(max_y/2 , max_x/2 -20 , "GAME OVER for Player %d !" , id) ;
                refresh() ; 
                sleep(50000) ;
            }
    }

    void printScore(string pos="left")
    {
        if(pos=="right")
            mvprintw(0 , 15, "Score = %d" , score) ; 
        mvprintw(0 , 0 , "Score = %d" , score) ; 
    }

    void handleMovementKeyPress(char ch )
    {
  
        if(keyUp==ch){ if(getDirection() !="down") move_snake("up") ; } 
        else if(keyDown==ch){ if(getDirection()!= "up")move_snake("down") ; } 
        else if(keyRight==ch){ if(getDirection()!="left")move_snake("right") ; } 
        else if(keyLeft==ch){ if(getDirection()!="right")move_snake("left") ; } 
        else return ; 
    }

    

    int getHeadX(void){return parts.at(parts.size()-1).x;}
    int getHeadY(void){return parts.at(parts.size()-1).y;}
    string getDirection(void){return snakeDirection; }

}; 




void draw_border_window(int  , int) ; 
void draw_food(void) ; 


void draw_border_window( int max_x , int max_y)
{
    for(int i =0 , j = 1 ; (i<max_x || j<max_y ) ; )
    {
        if(i<max_x)
            mvprintw(1,i++,"--") ;

        if(j<max_y)
            mvprintw(j++ , 0 , "|") ; 
    }

    max_x -=1 ; max_y-=1 ; 

    for(int i =max_x , j = max_y ; (i>0 || j>0) ; )
    {
        if(i>0)
            mvprintw( max_y , i--  , "--") ; 

        if(j>0)
            mvprintw(j--,max_x  , "|") ; 
        
    }
}





void charecter_code_testing_fun(void)
{
    //CHARACTER CODE TESTING FUNTION
    char prev_char  ,ch  ; 
    for(;;)
    {
        ch = getch() ; 
        if(ch!=ERR) prev_char = ch ; 
        mvprintw(10 , 10 , "Char = %c  : %d" , prev_char , prev_char) ; 
        usleep(10000) ; 
    }

}

void printSpeed(snake snk)
{
    mvprintw(0 , max_x-20 , "Speed= %lu" ,GameObj.getSpeed()) ; 
    refresh() ;
}


void Game::reset_max_screen()
{
    getmaxyx(stdscr , max_y , max_x ) ; 
}



void signalHandler(int code)
{
    cout<<"\nGracefull exit ! " ; 
    endwin() ; 
    logfile.close() ; 
    exit(0) ; 
}


int main(int argc , char * argv[]) 
{
    signal(SIGINT , signalHandler) ; 
    logfile.open ("logfileclient.log" , ios::out);

    srand(time(NULL)) ;
    system("clear") ;     
    string serverAddress  , player_name , player_sight ;  
    cout<<"Enter your name : " ; 

    cin.ignore() ; 
    getline(cin , player_name) ; 


    cout<<"Enter the IP address of the Controlling Server : "  ;
    cin>>serverAddress ; 

    cout<<"\nDo you want your snake to be displayed on your screen ? \n" ; 
    cout<<"1.Yes (You will see your snake on your screen and the same snake appears on the game controller) : " <<endl<<endl; 
    cout<<"2.No  (You will be able to control your snake by watching the game controller screen and your keyboard acts as a controller. This is the default choice ) : " <<endl;  
    cout<<endl<<"Enter choice (1 or 2) : " ; 
    cin>>player_sight;

    if(player_sight=="1")
        player_sight = "c" ;
    else
        player_sight = "s" ; 


    //Set some socket options
    GameObj.sock_obj.connectToServer(serverAddress , 8888) ; 
    
    GameObj.sock_obj.sendData("init~~" + player_name+"~~"+player_sight+"~~") ; 
    // GameObj.sock_obj.readData() ; 
   
    GameObj.initConsoleScreen("on") ; 
    //Initialize the snake object
    snake first_snake('A' , 'B' , 'C' , 'D' , 0 , player_name) ;
    first_snake.init_snake_on_screen() ; 


    char ch ; 
    for(;;)
    {
        GameObj.reset_max_screen() ; 

        if((ch = getch())!=ERR)
        {
            if(ch==27)
            {
                getch() ; // clear and reject 91 from buffer
                ch = getch() ;//Now store the actual value of arrow key pressed               getch() ; 
                string ch_string(1,ch) ; 
                GameObj.sock_obj.sendData(ch_string) ; 
                first_snake.handleMovementKeyPress(ch) ; 
            }
        }

        flushinp();
        clear() ;

        first_snake.move_snake(first_snake.getDirection()) ;

        printSpeed(first_snake) ; 
        GameObj.printFood() ;
        refresh() ;


        //Read for any incoming data from server ,,,
        GameObj.sock_obj.readData() ; 

        usleep(GameObj.getSpeed()) ;
    }

    refresh() ;

    sleep(10) ; 
    endwin() ; 
}
