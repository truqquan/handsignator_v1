#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <chrono>
#include <thread>

// Lấy file thư viện train model
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

// Khai báo hàm "lấy 1800 dữ liệu của input_buf đưa vào hàm run_classifier để phân loại"
static int get_signal_data(size_t offset, size_t length, float *out_ptr);
int get_signal_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        out_ptr[i] = (input_buf + offset)[i];
    }

    return EIDSP_OK;
}

// Khai báo các giá trị
float input_buf[1800] = {0};
std::string pre_dudoan = "";
std::chrono::milliseconds delay(1000);
const char* command = "";

int main(int argc, char **argv) {

    // Mở Serial
    int serialPort;
    char buffer_char[256];
    ssize_t bytesRead;

    // Open the serial port
    serialPort = open("/dev/ttyACM0", O_RDWR);

    if (serialPort == -1) {
        std::cout << "Error opening serial port." << std::endl;
        return 1;
    }

    // Set serial port parameters
    struct termios tty;
    if (tcgetattr(serialPort, &tty) != 0) {
        std::cout << "Error getting serial port attributes." << std::endl;
        close(serialPort);
        return 1;
    }

    tty.c_cflag &= ~PARENB;            // Disable parity
    tty.c_cflag &= ~CSTOPB;            // Use one stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                // Set data bits to 8
    tty.c_cflag &= ~CRTSCTS;           // Disable hardware flow control
    tty.c_cflag |= CREAD | CLOCAL;     // Enable receiver and ignore control lines

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Disable software flow control

    tty.c_lflag = 0;                   // Disable terminal processing

    tty.c_oflag = 0;                   // Disable output processing

    tty.c_cc[VMIN] = 1;                // Read at least 1 character
    tty.c_cc[VTIME] = 5;               // Wait for up to 0.5 seconds

    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        std::cout << "Error setting serial port attributes." << std::endl;
        close(serialPort);
        return 1;
    }

    //Khai báo các giá trị buffer, điều kiện, và biến đếm
    bool check_start = false;
    uint8_t k = 0, h = 0;               // k refer to index of buffer_temp, h refer to buffer_float
    char buffer_temp[256];
    float buffer_float[36] = {0};
    
    // EDGE_IMPULSE
    signal_t signal;            // Wrapper for raw input buffer
    ei_impulse_result_t result; // Used to store inference output
    EI_IMPULSE_ERROR res;       // Return code from inference

    // Update buffer with new data
    while (1)
    {
        bytesRead = read(serialPort, buffer_char, sizeof(buffer_char));
        if (bytesRead > 0) {
            // Process the received data
            for (ssize_t i = 0; i < bytesRead; ++i) {

                //std::cout << buffer_char[i]; // Each elements in the buffer
                
                if (buffer_char[i] == ','){
                    buffer_float[h] = std::strtof(buffer_temp, nullptr);
                    h++;
                    k = 0;
                    std::memset(buffer_temp, 0, sizeof(buffer_temp));
                }
                
                else if (buffer_char[i] == 10){    // 10 in ascii refer to enter to newline
    
                    buffer_float[h] = std::strtof(buffer_temp, nullptr);

                    std::memset(buffer_temp, 0, sizeof(buffer_temp));
                    
                    check_start = true;
                    k = 0;
                    h = 0;
                    i++;

                    for (uint16_t i = 0; i < 1764; i++){
                        input_buf[i] = input_buf[i+36];
                    }

                    uint8_t j = 0;
                    for (uint16_t i = 1764; i < 1800; i++){
                        input_buf[i] = buffer_float[j];
                        j++;
                    }
                    
                    // Calculate the length of the buffer
                    size_t buf_len = sizeof(input_buf) / sizeof(input_buf[0]);

                    // Make sure that the length of the buffer matches expected input length
                    if (buf_len != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
                        printf("ERROR: The size of the input buffer is not correct.\r\n");
                        printf("Expected %d items, but got %d\r\n", 
                                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, 
                                (int)buf_len);
                        return 1;
                    }

                    // Assign callback function to fill buffer used for preprocessing/inference
                    signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
                    signal.get_data = &get_signal_data;

                    // Perform DSP pre-processing and inference
                    res = run_classifier(&signal, &result, false);

                    // Print the prediction results (object detection)
                    #if EI_CLASSIFIER_OBJECT_DETECTION == 1
                        printf("Object detection bounding boxes:\r\n");
                        for (uint32_t i = 0; i < EI_CLASSIFIER_OBJECT_DETECTION_COUNT; i++) {
                            ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
                            if (bb.value == 0) {
                                continue;
                            }
                            printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n", 
                                    bb.label, 
                                    bb.value, 
                                    bb.x, 
                                    bb.y, 
                                    bb.width, 
                                    bb.height);
                        }

                        // Print the prediction results (classification)
                    #else
                        //printf("Predictions:\r\n");
                        
                        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
                            if ((result.classification[i].value > 0.99) && (pre_dudoan != ei_classifier_inferencing_categories[i]))
                            {
                                printf("  %s\n ", ei_classifier_inferencing_categories[i]);
                                printf("%.5f\r\n", result.classification[i].value);
                                pre_dudoan = ei_classifier_inferencing_categories[i];
                                if (pre_dudoan == "a")               {command = "mpg123 a.mp3";}
                                else if (pre_dudoan == "aa")         {command = "mpg123 aa.mp3";}
                                else if (pre_dudoan == "b")          {command = "mpg123 b.mp3";}
                                else if (pre_dudoan == "c")          {command = "mpg123 c.mp3";}
                                else if (pre_dudoan == "d")          {command = "mpg123 d.mp3";}
                                else if (pre_dudoan == "e")          {command = "mpg123 e.mp3";}
                                else if (pre_dudoan == "n")          {command = "mpg123 n.mp3";}
                                else if (pre_dudoan == "o")          {command = "mpg123 o.mp3";}
                                else if (pre_dudoan == "q")          {command = "mpg123 q.mp3";}
                                else if (pre_dudoan == "ten")        {command = "mpg123 ten.mp3";}
                                else if (pre_dudoan == "toi")        {command = "mpg123 toi.mp3";}
                                else if (pre_dudoan == "cam_on")     {command = "mpg123 cam_on.mp3";}
                                else if (pre_dudoan == "xin_chao")   {command = "mpg123 xin_chao.mp3";}
                                else if (pre_dudoan == "xin_loi")    {command = "mpg123 xin_loi.mp3";}
                                else if (pre_dudoan == "tuyet_voi")  {command = "mpg123 tuyet_voi.mp3";}
                                
                                //
                                system(command);
                                std::this_thread::sleep_for(delay);
                            }

                        }
                    #endif

                        // Print anomaly result (if it exists)
                    #if EI_CLASSIFIER_HAS_ANOMALY == 1
                        printf("Anomaly prediction: %.3f\r\n", result.anomaly);
                    #endif
                        
                    }                                    
                    
                
                else if (check_start == true){
                    buffer_temp[k] = buffer_char[i];
                    k++;
                }
            }
        }

        else if (bytesRead == -1) {
            std::cout << "Error reading from serial port." << std::endl;
            break;
        }

    }
}
