#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
extern char *__brkval;
static int __stacktop;
int stacktop() {
  char top; // Declare a variable that will
  // go on the top of the stack
  __stacktop = (int)&top; // Get its address
  return __stacktop;
}
int calculate_free_sram() {
  // this function calculates the amount of free memory the program has
  char top;
  return (int)&top - (int)__brkval;
}
// this sets up the creation of the up arrow
byte up_arrow[] = { B00100,
                    B01110,
                    B11111,
                    B00100,
                    B00100,
                    B00100,
                    B00000,
                    B00000
                  };
// this sets up the creation of the down arrow
byte down_arrow[] = {
  B00000,
  B00000,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};
typedef struct
{
  // this is is a global struct that will contain the channel information
  String channel;// this string will contain the channel description and the channel letter
  byte max_value;//contains the max value of the channel
  byte min_value;//contains the min value of the channel
  byte number_of_values;// this will contain how many values for a channel has been made
  byte recent_values[32];// this will contain the most recent 32 values for each channel
  byte average;// this will store the average of each channel
  byte channel_value;//this will contain the channel value
}  channel;// name of the structure is called channel
channel channel_array[13];//this array will store 13 disticnt channels
unsigned int channel_values_indexes_array[13];// this will store the indexes of the channels values that corrospond to the channels created
enum program_states {SYNCHRONISATION_STATE, MAIN_STATE};//fsm for program states
enum button_states {BUTTON_WAITING_TO_BE_PRESSED, BUTTON_WAITING_TO_BE_RELEASED };// fsm for buttons
/***********************these are the global variables for the program*******************/
unsigned int main_list_scrolling_index = 1;
unsigned int sublist_array_length = 0;
unsigned int right_sublist_scrolling_index = 1;
unsigned int left_sublist_scrolling_index = 1;
unsigned int first_description_index = 0;
unsigned int second_description_index = 0;
static unsigned long first_description_scrolling_time = 0;
static unsigned long second_description_scrolling_time = 0;
bool left_button_pressed = false;
bool right_button_pressed = false;
bool select_held = false;
/**********************************these are the functions used to change th global variables**************************/
void update_main_list_index(unsigned int index ) {
  main_list_scrolling_index = index; //this function manages the change of values for the main scrolling index
}
void update_right_sublist_scrolling_index(unsigned int index ) {
  right_sublist_scrolling_index = index; //this function manages the change of values for the right sublist scrolling index
}
void update_left_sublist_scrolling_index(unsigned int index) {
  left_sublist_scrolling_index = index; //this function manages the change of values for the right sublist scrolling index
}
void increment_first_description_index()// this function increments the first descriptions index by 1
{
  first_description_index = first_description_index + 1;
}
void increment_second_description_index()// this function increments the second descriptions index by 1
{
  second_description_index = second_description_index + 1;
}
void update_left_button_pressed(bool new_bool)// this function changes the bool left button pressed to a new value
{
  left_button_pressed = new_bool;
}
void update_right_button_pressed(bool new_bool)// this function changes the bool right button pressed to a new value
{
  right_button_pressed = new_bool;
}
void update_select_held(bool new_bool)// this function changes the bool select held to a new value
{
  select_held = new_bool;
}
/**************************Main code******************************************************/
void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  Serial.begin(9600);
  // setting up the lcd  and the serial monitor
}
void manage_scrolling_descriptions(unsigned int array_index, String first_description, String second_description)
{
  if ( select_held == false)
  {
    if (array_index == 1 )// if there is one channel
    { //Serial.println(F("DEBUG: only one value inputted into the system,LINE 108"));
      lcd.setCursor(10, 0);
      first_description.remove(0, 1);// removing the channel id from the description
      first_description.trim();
      if (first_description.length() <= 6)// if there is 6 or less characers in the description
      { //Serial.println(F("DEBUG: description length is less than or equal to 6,LINE 113"));
        lcd.print(first_description);// print the description
      }
      else {
        if ((millis() - first_description_scrolling_time) >= 500) {// if 500 ms has passed by
          first_description_scrolling_time = millis();
          scroll_description(first_description, first_description_index, 0);// call the function that scrolls the description
          increment_first_description_index();// call the function that increments the first channels descrpition  index
        }
        if (first_description_index >= first_description.length() + 1)// if first description has been scrolled
        { //Serial.println(F("DEBUG: finished scrolling,LINE 123"));
          first_description_index = 0;// set the index back to 0
        }
      }
    }
    if (array_index >= 2)// if there are more than 2 channels
    { // Serial.print(F("DEBUG: only two values inputted into the system,LINE 129"));
      first_description.remove(0, 1);// removing the channel id from the description
      second_description.remove(0, 1);
      if ((first_description.length() <= 6) & (second_description.length() <= 6) )// if the length of both channel descriptions are less than or equal to 6
      { //Serial.println(F("DEBUG: both channel descriptions are less than or equal to 6 characters,LINE 133"));
        // print both of those descriptions at their respective postitons
        lcd.setCursor(10, 0);
        lcd.print(first_description);// printing the first description
        lcd.setCursor(10, 1);
        lcd.print(second_description);// printing the second description
      }
      if ((first_description.length() > 6) & (second_description.length() <= 6))// if the first descriptions length is greater than 6 and the second descriptions length is less than or equal to 6
      { //Serial.println(F("DEBUG: first description needs to be scrolled, second one doesnt,LINE 141"));
        lcd.setCursor(10, 1);
        lcd.print(second_description);// print the second description
        if (millis() - first_description_scrolling_time >= 500)
        {
          first_description_scrolling_time = millis();
          scroll_description(first_description, first_description_index, 0);// call the function that scrolls the description  with the first description details
          increment_first_description_index();// increment the index
        }
        if (first_description_index >= first_description.length() + 1)
        {
          first_description_index = 0;// set the index to 0
        }
      }
      if ((second_description.length() > 6) & (first_description.length() <= 6)) {
        //Serial.println(F("DEBUG: second description needs to be scrolled, first one doesnt,LINE 156"));
        //second description needs to be scrolled
        lcd.setCursor(10, 0);
        lcd.print(first_description);// print the first description
        lcd.setCursor(10, 1);
        if (millis() - first_description_scrolling_time >= 500)// using the first description scrolling index as there is only one description that needs to be scrolled
        {
          first_description_scrolling_time = millis();
          scroll_description(second_description, second_description_index, 1);// calling the function for scrolling descriptions with the second channels details
          increment_second_description_index();
        }
        if (second_description_index >= second_description.length() + 1)
        {
          second_description_index = 0;
        }
      }
      if ((first_description.length() > 6) & (second_description.length() > 6)) {// both descriptions need to be scrolled
        // Serial.println(F("DEBUG: both descriptions need to be scrolled,LINE 173"));
        if (millis() - first_description_scrolling_time >= 500)
        {
          first_description_scrolling_time = millis();
          scroll_description(first_description, first_description_index, 0);// scroll description
          increment_first_description_index();
        }
        if (millis() - second_description_scrolling_time > 500)
        {
          second_description_scrolling_time = millis();
          scroll_description(second_description, second_description_index, 1);// scroll description
          increment_second_description_index();
        }
        if (first_description_index >= first_description.length() + 1) // when the whole description has been scrolled
        {
          first_description_index = 0;// set index to 0
        }
        if (second_description_index >= second_description.length() + 1)// when the whole description has been scrolled
        {
          second_description_index = 0;// set index to 0
        }
      }
    }
  }
}
bool check_for_white_spaces(String input)
{ // this function checks if there is any white spaces present in a string
  bool contains_white_space = false; // setting the intial value to false
  for (unsigned int index = 0; index < input.length(); index++)
  { // looping through each character of the string
    if (input[index] == ' ')
    { // if the character is a white space then set the bool to true and break out of the loop
      contains_white_space = true;
      break;
    }
  }
  return contains_white_space;// return the value of the bool
}
bool check_if_number(String value)
{ // this function checks if the value entered is an actual number
  bool is_number = true;
  for (unsigned int index = 0; index < value.length(); index++)
  {
    if (isdigit(value[index]) == 0) // its not a number
    {
      is_number = false; // its not a number
    }
  }
  return is_number;// returning the result
}
void loop() {
  static   unsigned int values_array_index = 0;// this will store the amount of values that have been entered into the program
  static program_states current_state = SYNCHRONISATION_STATE;// setting the state to the synchronisation state
  switch (current_state)
  {
    case SYNCHRONISATION_STATE:
      bool synchronisation_over;// bool check whether the program is done syncing
      lcd.setBacklight(5);// backlight purple
      synchronisation_over = synchronisation_phase();// store returned value from the syncronisation function
      if (synchronisation_over == true)
      { lcd.setBacklight(7);// set backlight to white
        current_state = MAIN_STATE;// change the fsm for the program states to the main state
      }
      break;
    case MAIN_STATE:
      button_control(values_array_index);// call the function that manages the buttons
      while (Serial.available())// waiting user input
      { //Serial.setTimeout(100);// input can come at 100ms
        bool set_max;// whether the max is being set or not
        bool valid_input = false; // bool to check if the input that was entered is valid or not
        String user_input = Serial.readStringUntil('\n');// storing the user input
        user_input.trim();
        if (user_input[0] == 'C' and user_input.length() >= 3)// if the user input starts with C and has a length greater than or equal to 3
        {
          if ((user_input[1] != ' ') & (isupper(user_input[1]) != 0)) { // checking if the channel id isnt a white space and that it is a capital letter from the alphabet
            valid_input = true; //its a valid input
            create_and_ammend_channel_description(user_input);// call the function that creates a channel
          }
        }
        if (user_input[0] == 'X' and (user_input.length() >= 3 and user_input.length() <= 5))// if the user input starts with X and is less than or equal to 5 characters and greater than or equal to 3
        { bool contains_white_space = check_for_white_spaces(user_input); // calling the function that checks for white spaces
          if (contains_white_space == false) { // if there are no white spaces
            set_max = true;//set the bool for setting a max value to true
            //Serial.println(F("DEBUG: setting channel max,LINE 256"));
            set_channel_max_and_min_value(user_input, set_max);//calling the value that sets min and max values for the channels
            valid_input = true; //its a valid input
            update_left_sublist_scrolling_index(0);
            update_right_sublist_scrolling_index(0);
          }
        }
        if (user_input[0] == 'N' and (user_input.length() >= 3 and user_input.length() <= 5 ))// if the user input starts with N and is less than or equal to 5 characters
        { bool contains_white_space = check_for_white_spaces(user_input); // calling the function that checks for white spaces
          if (contains_white_space == false) { // if there are no white spaces
            //Serial.println(F("DEBUG: setting channel min,LINE 266"));
            set_max = false;// not setting a max for a channel
            set_channel_max_and_min_value(user_input, set_max);//calling the value that sets min and max values for the channels
            valid_input = true; //its a valid input
            update_left_sublist_scrolling_index(0);
            update_right_sublist_scrolling_index(0);
          }
        }
        if (user_input[0] == 'V' and (user_input.length() >= 3 and user_input.length() <= 5 ))// if the user input starts with V and is less than or equal to 5 characters
        { bool contains_white_space = check_for_white_spaces(user_input); // checking for white spaces
          if (contains_white_space == false) { // if it doesnt contain white spaces
            bool is_in_range = false; // holds value if the value entered is between 0-255
            is_in_range = check_if_value_is_in_range(user_input); // storing the bool returned from the function that checks that the value is within the range
            if (is_in_range == true) // if its in range
            { //Serial.println(F("DEBUG: value inputted is within range,LINE 280"));
              valid_input = true; // its a valid input
              update_left_sublist_scrolling_index(0);
              update_right_sublist_scrolling_index(0);
              bool old_value_changed = true;
              old_value_changed = create_and_ammend_channel_value(user_input, values_array_index);// call the function that sets and updates channel values
              if (old_value_changed == false)// if the channel value is a new value
              { //Serial.println(F("DEBUG: new channel value inputted,LINE 287"));
                values_array_index = values_array_index + 1; // incrementing the  index for the values
              }
            }
          }
        }
        if (valid_input == false) // if the user input wasnt valid
        {
          Serial.println("ERROR: " + user_input + " LINE 295"); // print this error message alongside the user input
        }
        display_values(values_array_index);// call the function that displays the channel values of the main list of channels
      }
      determine_backlight(values_array_index );// call the function that decides what the backlight colour should be
      if ((left_button_pressed == false) & (right_button_pressed == false))// if the left and right button have not been pressed
      {
        display_main_list_descriptions(values_array_index);// calling the function thaat shows the descriptions of the main list channel values
      }
      if (left_button_pressed == true) { // if the left button has been pressed
        display_less_than_sublist_descriptions(values_array_index);// call the function that shows the descriptions of the sublist
      }
      if (right_button_pressed == true) { // if the right button has been pressed
        display_more_than_sublist_descriptions(values_array_index);// call the function that shows the descriptions of the sublist
      }
      break;
  }
}
void clear_description(unsigned int row)
{ // this function clears the section of the lcd screen where the descriptions will be scrolled across
  lcd.setCursor(10, row);
  lcd.print(F("       "));// clearing that section of the screen
}
void set_up_arrow()
{ // this function prints the up arrow
  lcd.createChar(0, up_arrow);// creating the up arrow character
  lcd.setCursor(0, 0);
  lcd.write((byte)0);// writing the up arrow to the lcd screen
}
void set_down_arrow()
{ // this function prints the down arrow
  lcd.createChar(1, down_arrow);// creating the down arrow character
  lcd.setCursor(0, 1);
  lcd.write((byte)1);// writing the down arrow to the lcd screen
}
void remove_up_arrow()
{ // this function removes the up arrow from the lcd screen
  lcd.setCursor(0, 0);
  lcd.print(F(" "));
}
void remove_down_arrow()
{ // this function removes the down arrow from the lcd screen
  lcd.setCursor(0, 1);
  lcd.print(F(" "));
}
void scroll_description(String description, unsigned int description_index, unsigned int row)
{ // this function is responsible for scrolling the actual channel descriptions
  char channel_description[description.length() + 1]; // creating a char array that has a length of the string description plus 1
  for (unsigned int index = 0; index < description.length(); index++) // looping through the descriptions length
  {
    channel_description[index] = description[index]; // adding each character of the description into the array
    if (index + 1 == description.length())
    {
      channel_description[description.length()] = '\0'; // setting the last charater  in the array to the terminating character
    }
  }
  if (row == 0) // if the row passed in as a parameter is equal to 0
  {
    clear_description(0);// clear the description area of the first row on the lcd screen
  }
  else {
    clear_description(1);// clear the description area of the second row on the lcd screen
  }
  lcd.setCursor(10, row);
  lcd.print(channel_description + description_index); // print the part of the description that corrosponds to the index
}
void  display_less_than_sublist_values(unsigned int values_array_index)
{
  // this function is responsible for displaying the channel values that are less than their channel value
  unsigned int number_of_values_below_min = return_number_of_values_below_min(values_array_index); // this will store the amount of channels that have a value below its min value
  unsigned int less_than_array[13];// this will store the indexes of the channels that are below their min value
  get_less_than_array(less_than_array, values_array_index); // this calling the function that updates the array
  if (number_of_values_below_min == 1)//if there is only one channel value bellow its min
  { one_channel_beyond_range(channel_array[less_than_array[0]].channel_value, channel_array[less_than_array[0]].channel, channel_array[less_than_array[0]].average);
    // call the function that shows the channel with the channel value and its corrosponding channel description
  }
  if (number_of_values_below_min >= 2)// if there are two or more channel values bellow their min value
  {
    unsigned int index;
    if (number_of_values_below_min == 2) // if there is two
    {
      index = 0; // set the index to 0
    }
    else
    {
      index = left_sublist_scrolling_index; // set the index to the scrolling index
    }
    // call this function passing in both channel values and their matching descriptions and averages
    more_than_one_channel_beyond_range( channel_array[less_than_array[index]].channel_value, channel_array[less_than_array[index]].channel, channel_array[less_than_array[index]].average, channel_array[less_than_array[index + 1]].channel_value, channel_array[less_than_array[index + 1]].channel, channel_array[less_than_array[index + 1]].average);
  }
  if (number_of_values_below_min >= 3)// if there are 3 or more channels
  {
    sublist_array_length = number_of_values_below_min;
    set_down_arrow();
    set_up_arrow();
  }
}
void  display_more_than_sublist_values(unsigned int values_array_index)
{ // this function is responsible for cheking which channel value should be displayed in the sublist
  unsigned int number_of_values_above_max = return_number_of_values_above_max(values_array_index); // this will store the amount of channels that are above their max
  unsigned int more_than_array[13];// this will store the indexes of the channels that have its channel value above its max value
  get_more_than_array(more_than_array, values_array_index); // updating the array to contain the values
  if (number_of_values_above_max == 1)//if there is only one channel value above its max
  { one_channel_beyond_range(channel_array[more_than_array[0]].channel_value, channel_array[more_than_array[0]].channel, channel_array[more_than_array[0]].average);
    // call the function that will print the channel value with the channel value and its corrosponding channel description and average
  }
  if (number_of_values_above_max >= 2)// if there are two or more channel values above their max value
  {
    unsigned int index;
    if (number_of_values_above_max == 2) // if there is two
    {
      index = 0; // set the index to 0
    }
    else
    {
      index = right_sublist_scrolling_index; // set the index to the scrolling index
    }
    // call this function passing in both channel values and their matching descriptions and averages
    more_than_one_channel_beyond_range(channel_array[more_than_array[index]].channel_value, channel_array[more_than_array[index]].channel, channel_array[more_than_array[index]].average, channel_array[more_than_array[index + 1]].channel_value, channel_array[more_than_array[index + 1]].channel, channel_array[more_than_array[index + 1]].average);
  }
  if (number_of_values_above_max >= 3)// if there is 3 or more values that fit the criteria
  {
    sublist_array_length = number_of_values_above_max;
    set_down_arrow();
    set_up_arrow();// show both arrows
  }
}
void display_more_than_sublist_descriptions(unsigned int values_array_index) {
  // this function is responsible for scrolling the correct description for the channel  that has its value above the max value
  unsigned int number_of_values_above_max = return_number_of_values_above_max(values_array_index); // stores the number of values that are above the max
  unsigned int more_than_array[13];// stores the indexes of the channels that have its channel value above the max
  get_more_than_array(more_than_array, values_array_index);
  if (number_of_values_above_max == 1)//if there is only one channel value above its max
  { manage_scrolling_descriptions(number_of_values_above_max, channel_array[more_than_array[0]].channel, "");// calling the function that scrolls the description
  }
  if (number_of_values_above_max >= 2)// if there are two or more channel values above their max value
  {
    unsigned int index;
    if (number_of_values_above_max == 2) // if there is two
    {
      index = 0; // set the index to 0
    }
    else
    {
      index = right_sublist_scrolling_index; // set the index to the scrolling index
    }
    // call this function passing in both channel values and their matching descriptions
    manage_scrolling_descriptions(number_of_values_above_max, channel_array[more_than_array[index]].channel, channel_array[more_than_array[index + 1]].channel);
  }
}
void display_less_than_sublist_descriptions(unsigned int values_array_index) {
  // this function calss other functions in order to ddisplay the channel descriptions for the channel values that are less than their min value
  unsigned int number_of_values_below_min = return_number_of_values_below_min(values_array_index); // storingthe amount of channels that have their channel value below their min value
  unsigned int less_than_array[13];// stores the index of the channels that have their values below their min value
  get_less_than_array(less_than_array, values_array_index); // updating the array
  if (number_of_values_below_min == 1)//if there is only one channel value bellow its min
  { manage_scrolling_descriptions(number_of_values_below_min, channel_array[less_than_array[0]].channel, "");// call the function that will print the description
  }
  if (number_of_values_below_min >= 2)// if there are two or more channel values below their min value
  {
    unsigned int index;
    if (number_of_values_below_min == 2) // if there is two
    {
      index = 0; // set the index to 0
    }
    else
    {
      index = left_sublist_scrolling_index; // set the index to the scrolling index
    }
    // call this function passing in both channel values and their matching descriptions
    manage_scrolling_descriptions(number_of_values_below_min, channel_array[less_than_array[index]].channel, channel_array[less_than_array[index + 1]].channel);
  }
}
void display_main_list_descriptions(unsigned int values_array_index) {
  // this function will determine which channel descriptions should be scrolled in the main list of channel values
  String first_description;
  String second_description;
  if (values_array_index == 1 )
  {
    first_description = channel_array[channel_values_indexes_array[0]].channel;//setting the first description
    manage_scrolling_descriptions(values_array_index, first_description, ""); // call the function that manages the scrolling
  }

  if (values_array_index >= 2)
  {
    if (values_array_index == 2)
    {
      first_description = channel_array[channel_values_indexes_array[0]].channel;// this is the first channel value in the array
      second_description = channel_array[channel_values_indexes_array[1]].channel; // this is the second channel value in the array
    }
    else
    {
      first_description = channel_array[channel_values_indexes_array[main_list_scrolling_index]].channel;//at the curent scrolling index
      second_description = channel_array[channel_values_indexes_array[main_list_scrolling_index + 1]].channel;// at the current scrolling index plus 1 - so its ne next channels descrption
    }
    manage_scrolling_descriptions(values_array_index, first_description, second_description); // call the function that manages the scrolling of the descriptions
  }
}
void display_values(unsigned int values_array_index)
{ // this function displays the channel values of channels in the main list
  byte number_of_digits;// this will store how many digits are in the channel value
  unsigned int index = 0;

  if (select_held == false)// if the select button is not beeing held
  { lcd.clear();
    if ((right_button_pressed == false) & (left_button_pressed == false)) // if the left button and the right button hasnt been pressed
    {
      if (values_array_index == 1)// if there is only one channel value in the program
      {
        lcd.setCursor(1, 0);
        number_of_digits = floor(log10(abs(channel_array[channel_values_indexes_array[0]].channel_value))) + 1; // calculating how many digits are in the channel value
        check_value_length(number_of_digits, channel_array[channel_values_indexes_array[0]].channel_value, channel_array[channel_values_indexes_array[0]].channel); // call the function that checks the channel length so that it could be formatted
        lcd.print(F(","));
        lcd.print(channel_array[channel_values_indexes_array[0]].average);// printing the channels average
      }
      if (values_array_index >= 2)// if there is two or more values
      {
        if (values_array_index == 2)
        {
          index = 0; // first channel in the array
        }
        else
        {
          index = main_list_scrolling_index; // set to the scrolling index so it can be scrolled through
          set_down_arrow();
          set_up_arrow();// setting both the arrows
        }
      }
      if (values_array_index >= 2)// if there is two values or more
      {
        bubble_sort(values_array_index);// call the function that sorts the channels in alphabetical order
        lcd.setCursor(1, 0);
        number_of_digits = floor(log10(abs(channel_array[channel_values_indexes_array[index]].channel_value))) + 1; // calculating the number of digits
        check_value_length(number_of_digits, channel_array[channel_values_indexes_array[index]].channel_value, channel_array[channel_values_indexes_array[index]].channel); // check the  number of characters
        lcd.print(F(","));
        lcd.print(channel_array[channel_values_indexes_array[index]].average);// printing the average
        lcd.setCursor(1, 1);
        number_of_digits = floor(log10(abs(channel_array[channel_values_indexes_array[index + 1]].channel_value))) + 1;
        check_value_length(number_of_digits, channel_array[channel_values_indexes_array[index + 1]].channel_value, channel_array[channel_values_indexes_array[index + 1]].channel); // checking the number of characters
        lcd.print(F(","));
        lcd.print(channel_array[channel_values_indexes_array[index + 1]].average); // printing the average
      }
    }
    if (left_button_pressed == true) // if the left button has been pressed
    { lcd.clear();
      display_less_than_sublist_values(values_array_index);// call the function that displays the channel values of the appropriate sublist
    }
    if (right_button_pressed == true)
    { lcd.clear();
      display_more_than_sublist_values(values_array_index);// call the function that displays the channel values of the appropriate sublist
    }
  }
}
bool synchronisation_phase()
{ // this function carries out the synchronisation phase of the program
  unsigned long previous_time = 0; //storing the time
  bool sync_done = false;
  if (millis() - previous_time >= 1000)//if the elapsed time is greater than or equal to 1 second
  {
    Serial.print(F("Q"));// print Q
    previous_time = millis();
    if (Serial.readString()[0] == 'X')// if the user input starts with a capital X
    { //Serial.println(F("DEBUG: sychronisation phase completed,LINE 557"));
      Serial.println(F("UDCHARS,FREERAM,HCI,RECENT,NAMES,SCROLL"));// print this
      sync_done = true;// set the bool to true since the sync stage has now been completed
    }
  }
  return sync_done;// return the value of the bool
}
void create_and_ammend_channel_description(String channel_name_and_description)
{ // this function creates channels and updates channel descriptions
  static unsigned int channel_array_index = 0;// index for which postion of the array we should store the channel to
  bool new_channel = true; // bool for a new channel being created
  channel_name_and_description.remove(0, 1);// removing the 'C' character from the user input
  if (channel_name_and_description.length() > 16)// if the  length of the user input is greater than 16 characters
  {
    channel_name_and_description.remove(16, channel_name_and_description.length()); // remove the extra characters as the channel description can not be longer than 15 characters ( first character of the string is the channel id)
  }
  //Serial.println("DEBUG: the channel description is : "+ channel_name_and_description);
  for (unsigned int index = 0; index < 13; index++) {
    if (channel_array[index].channel[0] ==  channel_name_and_description[0]) // if the channel id is the same as the channel id in the user input
    { //Serial.println(F("DEBUG: original channel description found,LINE 576"));
      channel_array[index].channel = channel_name_and_description;// set that channel description to the one the user entered
      new_channel = false; // since we are updating a channel description we set this bool to false
      break;
    }
  }
  if (new_channel == true) { // if we are creating a new channel
    //Serial.println(F("DEBUG: new channel being created,LINE 583"));
    if (channel_array_index != 13) // if there is not already 13 channels in the program
    {
      channel_array[channel_array_index].channel = channel_name_and_description;// set the channel description for the new channel
      channel_array[channel_array_index].max_value = 255;//set the max for the new channel to the default value
      channel_array[channel_array_index].min_value = 0;// set the min for the new channel to the default value
      channel_array_index++;// increment the channel array index so that a new channel can be created
    }
    else
    {
      Serial.println(F("DEBUG: NO MORE CHANNELS PERMITTED, 13 CHANNELS HAVE BEEN REACHED,LINE 593"));// if there are 13 channels already print this
    }
  }
}
void set_channel_max_and_min_value(String value, bool setting_max)
{ // this value is responsible for setting the max and min value for a channel
  bool is_valid = false;
  value.remove(0, 1);// removing the first character from the value so we are just left with the integer and channel id (first character is character 'X' OR 'N' depending on whether setting a max or a min)
  for (unsigned int index = 0; index < 13; index++) { // looping through each of the channels
    if ((channel_array[index].channel)[0] == value[0])// if the channel id from the user input is matching one in the array of channels
    {
      value.remove(0, 1);// removing the channel id
      bool is_number = check_if_number(value); // calling the function that checks if the value is an actual number
      if (is_number == true) {
        if ((value.toInt() >= 0) & (value.toInt() <= 255)) // if the value is within the range of 0-255
        { byte numerical_value;
          if (setting_max == true) {
            // if we are setting the max for the channel
            numerical_value = value.toInt(); //storing the value
            channel_array[index].max_value = numerical_value;// set that channel max value to the users input
            is_valid = true; // its a valid input
            break;
          }
          else
          { // setting a min
            numerical_value = value.toInt(); // storing the value
            channel_array[index].min_value = numerical_value;// set that channel min value to the users input
            is_valid = true; // its a valid input
            break;
          }
        }
      }
    }
  }
  if (is_valid == false) // if the input wasnt valid
  {
    Serial.println("ERROR: " + value + " LINE 629"); // print this error message alongside the user input
  }
}
bool check_if_value_is_in_range(String value)
{ // this function is used to determine if the channel value entered by the user is within the range of 0-255
  value.remove(0, 2);// removing the 'V' and channel id from the user input
  bool is_number = check_if_number(value); // calling the function that checks if the value is an actual number
  if (is_number == true) {
    if ( (value.toInt() >= 0) &  (value.toInt() <= 255)) // if its in range
    { //Serial.println(F("DEBUG: inputted value is in range,LINE 638"));
      return true;
    }
    else
    { // if its not in range
      //Serial.println(F("DEBUG: inputted value is not in range,LINE 643"));
      return false;
    }
  }
  else {
    // its not a valid number
    return false;
  }
}
bool create_and_ammend_channel_value(String inputted_channel_value, unsigned int values_array_index)
{ // this function is used to create and update channel values for the channels that have been inputted into the
  inputted_channel_value.remove(0, 1);// removing the "v" character from the user input
  bool original_channel_value_found = false;// bool for checking if the original channel value has been found
  bool valid_input = false;// if an input is valid
  int channel_value_position = -1;
  int corrosponding_channel_index = -1;
  for (unsigned int index = 0; index < 13; index++)
  {
    if (channel_array[index].channel[0] == inputted_channel_value[0])
    { //if there is a matching channel id to the one in the user input
      valid_input = true;// set the bool to true as its valid
      corrosponding_channel_index = index; // store what index the corrosponding channel is stored at
      break;
    }
  }
  for (unsigned int index = 0; index < 13; index++)
  {
    if ((channel_array[index].number_of_values != 0) & (channel_array[index].channel[0] == inputted_channel_value[0])) // if there is an existing channel value
    {
      channel_value_position = index;// store the index of where it is
      original_channel_value_found = true;// set the bool for the original channel found to true
      break;
    }
  }
  if (valid_input == false)
  {
    Serial.println("ERROR: " + inputted_channel_value + " LINE 679"); // print this error message alongside the user input
  }
  if ((valid_input == true ) & (original_channel_value_found == true) & (channel_value_position != -1) )
  { // if it was a valid input  and the channel value is not equal to -1
    //Serial.println(F("DEBUG: editing the original channel value,LINE 683"));
    inputted_channel_value.remove(0, 1);// removing the channel id
    byte value =  inputted_channel_value.toInt(); // storing the channel value
    if (channel_array[channel_value_position].number_of_values != 32) // if there hasnt been 32 values inputted for this channel
    {
      channel_array[channel_value_position].channel_value = value;// set the channel value at the respective channel array index
      channel_array[channel_value_position].recent_values[channel_array[channel_value_position].number_of_values] = value; // storing the channel value as the least most recent value
      channel_array[channel_value_position].number_of_values = channel_array[channel_value_position].number_of_values + 1; // incrementing the amount of channel values inputted for this channel
      double total = 0; // this will store the total
      for (unsigned int index = 0; index < channel_array[channel_value_position].number_of_values; index++) {
        //getting all of the values that have been stored
        //Serial.print(F("DEBUG: Value --> "));
        //Serial.println(channel_array[channel_value_position].recent_values[index]);
        total = total + channel_array[channel_value_position].recent_values[index]; // adding all the values to the total
      }
      channel_array[channel_value_position].average = round(total / channel_array[channel_value_position].number_of_values); // calculating the average rounded to the nearest integer
    }
    else
    {
      Serial.println(F("DEBUG:There is already 32 values for this channel, least recent values will be replaced,LINE 702"));
      channel_array[channel_value_position].channel_value = value;// storing the channel value
      for (unsigned int index = 0; index < 31; index++)  {// was 32 for both
        channel_array[channel_value_position].recent_values[index] = channel_array[channel_value_position].recent_values[index + 1];// moving each value of the array down so the least recent value is removed from the array
      }
      channel_array[channel_value_position].recent_values[31] = value ; // the last value in the array will be the new value just entered
      double total = 0; // this will store the total
      for (unsigned int index = 0; index < 32; index++) {
        // Serial.print(F("DEBUG: VALUE : "));Serial.println(channel_array[channel_value_position].recent_values[index]);
        total = total + channel_array[channel_value_position].recent_values[index]; //adding each of the recent values to the total
      }
      channel_array[channel_value_position].average = round(total / 32); //calculating the channels average
    }
    return original_channel_value_found;// return the value of this bool
  }
  else if ((valid_input == true) & (original_channel_value_found == false) & (channel_value_position == -1))
  { //if the input was valid and the original value has not been found and the channel position is still -1 - so it is a new channel value
    //Serial.println(F("DEBUG: creating a new channel value,LINE 719"));
    inputted_channel_value.remove(0, 1);// removing the channel id
    byte value = inputted_channel_value.toInt(); // storing the channel value
    channel_array[corrosponding_channel_index].channel_value = value; // setting the channel value
    channel_values_indexes_array[values_array_index] = corrosponding_channel_index; // storing the index into the array that contains all the channel value indexes
    channel_array[corrosponding_channel_index].recent_values[0] = value; //setting the value just inputted to the least recent value
    channel_array[corrosponding_channel_index].number_of_values = channel_array[corrosponding_channel_index].number_of_values + 1; // incrementing the number of values that have been made for this channel by one
    channel_array[corrosponding_channel_index].average = value; //setting the average to the number just inputted as any number divided by itself is the number
    return original_channel_value_found;
  }
  else
  {
    return true;
  }
}
void up_button_control(unsigned int values_array_index)
{ //this function is called when the up button is pressed
  if ((left_button_pressed == false) & (right_button_pressed == false)) // if the left and right button has not been pressed
  {
    if ((int)main_list_scrolling_index  - 1 >= 0)// if the last element of the list hasnt been reached
    {
      update_main_list_index(--main_list_scrolling_index);// deecrement the main srolling index
      display_values(values_array_index);// display the values
    }
  }
  if (left_button_pressed == true) // if the left button has been pressed
  {
    if ((int)left_sublist_scrolling_index - 1 >= 0)// checking if we can still scroll through the list
    {
      update_left_sublist_scrolling_index(--left_sublist_scrolling_index);// decrement the scrolling index
      display_values(values_array_index);
    }
  }
  if (right_button_pressed == true)
  {
    if ((int)right_sublist_scrolling_index - 1 >= 0)// checking if we can still scroll through the list
    {
      update_right_sublist_scrolling_index(--right_sublist_scrolling_index);// decrement the scrolling index
      display_values(values_array_index);
    }
  }
}
void down_button_control(unsigned int values_array_index)
{ //this function carries out the operations when the down button is pressed
  if ((left_button_pressed == false) & (right_button_pressed == false))
  { // both left and right button has not been pressed
    if (main_list_scrolling_index   + 2 < values_array_index)// if the last element in the list has bot been reached yet
    {
      update_main_list_index(++main_list_scrolling_index);// increment the scrolling index
      display_values(values_array_index);// calling the function that displays the channel values
    }
  }
  else if (left_button_pressed == true )
  { // if the left button has been pressed
    if (left_sublist_scrolling_index + 2 < sublist_array_length)// if the last element in the list has bot been reached yet
    {
      update_left_sublist_scrolling_index(++left_sublist_scrolling_index);// increment the index for scrolling in the sublist
      display_values(values_array_index);// display the values
    }
  }
  else if (right_button_pressed == true)
  {
    if (right_sublist_scrolling_index + 2 < sublist_array_length)// checking if we can scroll through the list
    {
      update_right_sublist_scrolling_index(++right_sublist_scrolling_index);// increment the index for scrolling in the sublist
      display_values(values_array_index);// display the values
    }
  }
}
void select_button_held()
{ // this function is called when the select button is held for longer than a second
  int amount_of_free_sram;
  amount_of_free_sram = calculate_free_sram(); // storing the value returned from the function that calculates the amount of free  memory
  remove_up_arrow();
  remove_down_arrow();// removing  both the arrows
  lcd.setCursor(4, 0);
  lcd.print(F("F122571"));// printing my student ID
  lcd.setCursor(4, 1);
  lcd.print(amount_of_free_sram);// printing the amount of free memory
  lcd.setBacklight(5);// set backlight to purple
}
void  select_button_released( unsigned int values_array_index)
{ // this function is called when the select button has been released
  display_values(values_array_index);// display the channel values
}
void manage_arrow_control_for_scrolling(unsigned int values_array_index)
{
  // this function controlls which arrows should bee displayed
  if ((main_list_scrolling_index == 0) & ((right_button_pressed == false) & (left_button_pressed == false)) )
  { //if the last value in the array is reached and the left and right button hasnt been pressed
    remove_up_arrow();// remove the up arrow
  }
  if ((main_list_scrolling_index  + 2 == values_array_index) & ((right_button_pressed == false) & (left_button_pressed == false)))
  { // if the last value has been reached and left/right have not been pressed
    remove_down_arrow();// remove the down arrow
  }
  if ((left_sublist_scrolling_index == 0) & (left_button_pressed == true))
  { // if th left arrow is pressed and the first value in the sublist has been reached then remove the up arrow
    remove_up_arrow();
  }
  if ((left_sublist_scrolling_index == sublist_array_length - 2 ) & (left_button_pressed == true))
  { // if th left arrow is pressed and the last value in the sublist has been reached then remove the down arrow
    remove_down_arrow();
  }

  if ((right_sublist_scrolling_index == 0) & (right_button_pressed == true))
    // removing the up arrow
  { // same as above , only works for when the right button has been pressed
    remove_up_arrow();
  }
  if ((right_sublist_scrolling_index == sublist_array_length - 2) & (right_button_pressed == true))
  { // removing the  down arrow
    remove_down_arrow();
  }
}
void button_control (unsigned int values_array_index)
{ // this function controlls what happens when each botton is pressed and released
  static enum button_states button_state =  BUTTON_WAITING_TO_BE_PRESSED;// setting the fsm for the button states to waiting to be pressed
  static  unsigned int which_button_pressed; // Store which button was pressed
  static unsigned long press_time; // Store the time the button was pressed
  static  unsigned int last_button_pressed = 0; // Store for the last button press
  static  long press_time_select = 0; // store time the select button was held for
  static  unsigned int times_left_button_pressed;// stores how many times the left button is pressed
  static  unsigned int times_right_button_pressed;// stores how many times the right button is pressed
  static bool  up_button_pressed;// stores the bool if the up button is pressed
  static bool  down_button_pressed;// stores the bool if the down  button is pressed
  manage_arrow_control_for_scrolling(values_array_index);// calling the function that manages which arrows should be shown
  switch (button_state) {
    case BUTTON_WAITING_TO_BE_RELEASED:
      if (millis() - press_time >= 150) {
        press_time = millis(); // Reset pressed time to curent time
        if ((which_button_pressed == BUTTON_UP) & (up_button_pressed == false )) {
          if (values_array_index > 2)
          {
            // if the up button is pressed and there is more than two channel values in the system
            up_button_control(values_array_index);// call the function that manages the up arrow being pressed
            up_button_pressed = true; // set this bool to true so that it doesnt continously scroll
          }
        }
        else if ((which_button_pressed == BUTTON_DOWN) & (down_button_pressed == false ))
        {
          if (values_array_index > 2)
          {
            // if the down button is pressed and there is more than two channel values in the system
            down_button_control(values_array_index);// call the function that manages the down arrow being pressed
            down_button_pressed = true; // set this bool to true so that it doesnt continously scroll
          }
        }
        else if (which_button_pressed == BUTTON_LEFT)
        { // if the left button has been pressed
          if (times_left_button_pressed == 0) {
            times_left_button_pressed = times_left_button_pressed + 1; // add one to this variable
          }
        }
        else if (which_button_pressed == BUTTON_RIGHT)
        {
          // if the right button is pressed then
          if (times_right_button_pressed == 0) {

            times_right_button_pressed = times_right_button_pressed + 1; // add one
          }
        }
        else if (which_button_pressed == BUTTON_SELECT)//if select has been pressed
        { // if the select button is pressed
          if ( millis() - press_time_select >= 1000)
          { // if its been held for a second or more
            press_time_select = millis();
            if (select_held == false) { // clearing the screen
              lcd.clear();
            }
            update_select_held(true);// set the bool to true
            select_button_held();// calling the function that carries out the tasks when the select button is held
          }
        }
      }
      else {
        unsigned int button = lcd.readButtons();// storing the button pressed
        unsigned int button_released = ~button & last_button_pressed;// button that wasnt just pressed
        last_button_pressed = button; // Saving the button
        if (button_released & which_button_pressed) {
          up_button_pressed = false;
          down_button_pressed = false;
          // setting these bools to false as the up and down is released
          if ((which_button_pressed == BUTTON_LEFT) & (times_left_button_pressed == 1))
          { // if the left button has been pressed and released once :
            times_right_button_pressed = 0; // set the times the right button is pressed to 0
            update_left_button_pressed(true);// set left pressed to true
            update_right_button_pressed(false);// set right pressed to false
            display_values(values_array_index);
            //Serial.println(F("DEBUG: left button released once,LINE 908"));
          }
          if ((which_button_pressed == BUTTON_LEFT) & (times_left_button_pressed == 2))
          { // this is when the left button has been released for the second time
            times_left_button_pressed = 0; // set to 0
            update_left_button_pressed(false);// set left being pressed to false
            lcd.clear();
            display_values(values_array_index);// display the main list of channel values
            //Serial.println(F("DEBUG: left button released twice,LINE 916"));
          }
          if ((which_button_pressed == BUTTON_RIGHT) & (times_right_button_pressed == 1))
          { //when the right button has been released once
            update_left_button_pressed(false);
            update_right_button_pressed(true);// set right to true and left to false
            times_left_button_pressed = 0;
            display_values(values_array_index);
            //Serial.println(F("DEBUG: right button released once,LINE 924"));
          }
          if ((which_button_pressed == BUTTON_RIGHT) &  (times_right_button_pressed == 2))
          { // when the right button is released for the second time
            times_right_button_pressed = 0;
            update_right_button_pressed(false);
            display_values(values_array_index); //display the main list of channel values
            //Serial.println(F("DEBUG: right button released twice,LINE 931"));
          }
          if ((which_button_pressed == BUTTON_SELECT) & (select_held == true ))// if select was the last button pressed and select held is true
          { update_select_held(false);// set select held to false
            select_button_released(values_array_index);// call the function that does the tasks when the select button has been released
            //Serial.println(F("DEBUG: select button no longer held,LINE 936"));
          }
          button_state = BUTTON_WAITING_TO_BE_PRESSED;
        }
      }
      break;
    case BUTTON_WAITING_TO_BE_PRESSED :
      unsigned int button = lcd.readButtons(); // We are looking for buttons that were NOT pressed
      // and are pressed now.
      // Logic is now AND NOT last_time
      unsigned int pressed = button & ~last_button_pressed;
      last_button_pressed = button;
      if (pressed & (BUTTON_UP | BUTTON_DOWN | BUTTON_SELECT | BUTTON_LEFT | BUTTON_RIGHT)) { // checking for these buttons
        if (pressed & (BUTTON_UP) & (up_button_pressed == false))
        { // if the up button is pressed
          if (values_array_index > 2)
          {
            // if there is atleast 3 values in the system
            up_button_control(values_array_index);// call this function
            up_button_pressed = true; // set up pressed to true
          }
        }
        else if (pressed & (BUTTON_DOWN) &  (down_button_pressed == false))
        {
          // if the down button is pressed
          if (values_array_index > 2)
          { // if there is atleast 3 channel values
            down_button_control(values_array_index);// call the down button function
            down_button_pressed = true; // set down to true
          }
        }
        else if (pressed & (BUTTON_LEFT))
        {
          times_left_button_pressed = times_left_button_pressed + 1; // increment the times that the left button has been pressed
        }
        else if (pressed & (BUTTON_RIGHT))
        {
          times_right_button_pressed = times_right_button_pressed + 1; // increment the times that the right button has been pressed
        }
        which_button_pressed = pressed;
        press_time = millis();// resseting the times
        press_time_select = millis();
        button_state = BUTTON_WAITING_TO_BE_RELEASED;// changing the state to the waiting to be released sate
      }
      break;
  }
}
/************These functions print the channel values in a right justified manner**********/
void format_four_character_channel_value(byte value, String channel)
{ // this function prints a 4 character value
  lcd.print(channel[0]);
  lcd.print(value);
}
void format_three_character_channel_value(byte value, String channel)
{ //this function prints a 3 character value
  lcd.print(channel[0]);// channel id
  lcd.print(F(" "));
  lcd.print(value);
}
void format_two_character_channel_value(byte value, String channel)
{ //this function prints a 2 character value in a right justified manner
  lcd.print(channel[0]);// channel id
  lcd.print(F("  "));
  lcd.print(value);
}
/*************************************************************/
void check_value_length(byte channel_value_length, byte channel_value, String channel_name)
{ // this function checks for which length the channel value is
  if (channel_value_length == 3) // if there is 3 digits in the channel value
  {
    format_four_character_channel_value(channel_value, channel_name); // call this function
  }
  if (channel_value_length == 2) // if it has two digits
  {
    format_three_character_channel_value(channel_value, channel_name); // call this function
  }
  if ((channel_value_length == 1) | (channel_value_length == 0)) // if it has 1 digit or it has zero digits- when a channel value of 0 is entered,it has 0 digits
  {
    format_two_character_channel_value(channel_value, channel_name); // call this function
  }

}
void one_channel_beyond_range(byte channel_value, String channel_description, byte average)
{ // this function is called when there is a value thats beyond its range
  byte number_of_digits;
  lcd.setCursor(1, 0);
  number_of_digits = floor(log10(abs(channel_value))) + 1; // number of digits in the channel value
  check_value_length(number_of_digits, channel_value, channel_description); // calling the function that checks the length and formats the channel value
  lcd.print(F(","));
  lcd.print(average); // print the average for that channel
}
void more_than_one_channel_beyond_range(byte first_channel_value,  String first_channel_description, byte first_average, byte second_channel_value, String second_channel_description, byte second_average)
{ // does the same process as the one outlined above but does it if there is 2 or more channel values beyond their range
  byte number_of_digits;
  lcd.setCursor(1, 0);
  number_of_digits = floor(log10(abs(first_channel_value))) + 1;
  check_value_length(number_of_digits, first_channel_value, first_channel_description);
  lcd.print(F(","));
  lcd.print(first_average);// prints the average
  lcd.setCursor(1, 1);
  number_of_digits = floor(log10(abs(second_channel_value))) + 1;
  check_value_length(number_of_digits, second_channel_value, second_channel_description); // checking the  length of the value
  lcd.print(F(","));
  lcd.print(second_average);
}
unsigned int return_number_of_values_below_min(unsigned int values_array_index)
{ // this function returns the number of values that are below the min value
  unsigned int number_of_values_below_min = 0; // stores the number of values
  for (unsigned int index = 0; index < values_array_index; index++)
  {
    if (channel_array[channel_values_indexes_array[index]].number_of_values != 0) // if there is a channel value for that channel
    {
      if ((channel_array[channel_values_indexes_array[index]].channel_value <  channel_array[channel_values_indexes_array[index]].min_value) & (channel_array[channel_values_indexes_array[index]].max_value > channel_array[channel_values_indexes_array[index]].min_value ))
      { //if there is a value thats below its min and the max value is greater than the min value
        number_of_values_below_min = number_of_values_below_min + 1;// increment the number of channel values below the min by 1
      }
    }
  }
  return number_of_values_below_min;// return the number
}
unsigned int return_number_of_values_above_max(unsigned int values_array_index)
{ // this function returns how many values are above their max value
  unsigned int number_of_values_above_max = 0;// this will store the amount of values that are above their max
  for (unsigned int index = 0; index < values_array_index; index++)
  {
    if (channel_array[channel_values_indexes_array[index]].number_of_values != 0) // if there is a channel value for that channel
    {
      if ((channel_array[channel_values_indexes_array[index]].channel_value >  channel_array[channel_values_indexes_array[index]].max_value) & (channel_array[channel_values_indexes_array[index]].max_value > channel_array[channel_values_indexes_array[index]].min_value ))
      { // if there is a value thats above its max and the max value is greater than the min value
        number_of_values_above_max = number_of_values_above_max + 1;// increment the number of channel values above the max by 1
      }
    }
  }
  return number_of_values_above_max;// returning the value
}
void get_less_than_array(unsigned int less_than_array[], unsigned int values_array_index)
{ // this function adds the indexes of the channels that have a value less than its min value
  unsigned int number_of_values_below_min = 0; // LEFT BUTTON
  for (unsigned int index = 0; index < values_array_index; index++)
  {
    if (channel_array[channel_values_indexes_array[index]].number_of_values != 0) // if there is a channel value for that channel
    {
      if ((channel_array[channel_values_indexes_array[index]].channel_value <  channel_array[channel_values_indexes_array[index]].min_value) & (channel_array[channel_values_indexes_array[index]].max_value > channel_array[channel_values_indexes_array[index]].min_value ))
      { //if there is a value thats below its min and the max value is greater than the min value
        less_than_array[number_of_values_below_min] = channel_values_indexes_array[index]; //store the index of that channel value into this array
        number_of_values_below_min = number_of_values_below_min + 1;// increment the number of channel values below the min by 1
      }
    }
  }
}
void get_more_than_array(unsigned int more_than_array[], unsigned int values_array_index)
{ // this function adds the indexes of the channels that have their channel value above its max value
  unsigned int number_of_values_above_max = 0; // LEFT BUTTON
  for (unsigned int index = 0; index < values_array_index; index++)
  {
    if (channel_array[channel_values_indexes_array[index]].number_of_values != 0) // if there is a channel value for that channel
    {
      if ((channel_array[channel_values_indexes_array[index]].channel_value > channel_array[channel_values_indexes_array[index]].max_value) & (channel_array[channel_values_indexes_array[index]].max_value > channel_array[channel_values_indexes_array[index]].min_value ))
      { //if there is a value thats above its max and the max value is greater than the min value
        more_than_array[number_of_values_above_max] = channel_values_indexes_array[index]; //store the index of that channel value into this array
        number_of_values_above_max = number_of_values_above_max + 1;// increment the number of channel values
      }
    }
  }
}
void determine_backlight(unsigned int values_array_index)
{ // this function determines what backlight colour the lcd should be set to
  bool any_greater_than = false;//bool stores whether there is values greater than their max
  bool any_less_than = false;//bool stores whether there is values greater than their max
  if (select_held == false)// if the select button is not beeing held
  {
    for (unsigned int index = 0; index < values_array_index; index++)
    {
      if (channel_array[channel_values_indexes_array[index]].number_of_values != 0) // if there is a channel value for that channel
      {
        if ((channel_array[channel_values_indexes_array[index]].channel_value >  channel_array[channel_values_indexes_array[index]].max_value) & (channel_array[channel_values_indexes_array[index]].max_value > channel_array[channel_values_indexes_array[index]].min_value ))
        { // if there is a value thats above its max and the max value is greater than the min value
          any_greater_than = true;// set this bool to true
        }
        if ((channel_array[channel_values_indexes_array[index]].channel_value <  channel_array[channel_values_indexes_array[index]].min_value) & (channel_array[channel_values_indexes_array[index]].max_value > channel_array[channel_values_indexes_array[index]].min_value ))
        { //if there is a value thats bellow its min and the max value is greater than the min value
          any_less_than = true;// set this bool to true
        }
      }
    }
    if ((any_greater_than == true) & (any_less_than != true))
    { // if there is a value above its max and there is no value bellow their min
      lcd.setBacklight(1);// set backlight to red
      //Serial.println(F("DEBUG: RED,LINE 1124"));
    }
    if ( (any_greater_than != true) & (any_less_than == true))
    { // if there is values that are less than its min and there are no channel values that are above their max value
      lcd.setBacklight(2);//set the backlight to green
      //Serial.println(F("DEBUG: GREEN, LINE 1129"));
    }
    if ((any_greater_than == true) & (any_less_than == true))
    { // if there values above the max and bellow the min
      lcd.setBacklight(3);//yellow
      //Serial.println(F("DEBUG: YELLOW,LINE 1134"));
    }
    if ( (any_greater_than == false) & (any_less_than == false))// if no values are beyond their min or max
    {
      lcd.setBacklight(7);//set backlight to white
      //Serial.println(F("DEBUG: WHITE, LINE 1139"));
      if ((left_button_pressed == true)  | (right_button_pressed == true))
      { //clear the screen if the left or right button has been pressed
        lcd.clear();
      }
    }
  }
}
void bubble_sort(unsigned int values_array_index)
{ // this function sorts the channels into alphabetical order
  boolean Swaps = true;// setting the boolean to true
  while (Swaps == true)// while swaps is true
  {
    Swaps = false;// set swaps to false, breaks out of the loop
    for (unsigned int index = 0; index < values_array_index - 1; index++)
      // repeat this in range from  0 to the length of the array - 1
    {
      if ((channel_array[channel_values_indexes_array[index]].channel)[0] > (channel_array[channel_values_indexes_array[index + 1]].channel)[0])
        // if the array value's channel id in a position is greater than the channel id in the next position
      {
        // THIS swaps the values
        channel greater_channel = channel_array[channel_values_indexes_array[index]]; // store the channel at the index-th postion
        channel lesser_channel = channel_array[channel_values_indexes_array[index + 1]]; // store the channel in the index+1 th position
        channel temporaryHolder = lesser_channel; // store the channel in a temporary place
        channel_array[channel_values_indexes_array[index + 1]] = greater_channel;
        channel_array[channel_values_indexes_array[index]] = temporaryHolder; // swapping the channel positions
        Swaps = true;// setting the value to true so it can progress with the loop
      }
    }
  }
}
