/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2023
 * Modified Code (C) Copyright Avnet
 *　
 *  *1 DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
/***********************************************************************************************************************
 * File Name    : parkinglot_detection.cpp
 * Version      : 1.0
 * Description  : DRP-AI TVM[*1] Application Example
 ***********************************************************************************************************************/
/*****************************************
 * includes
 ******************************************/
#include <builtin_fp16.h>
#include <fstream>
#include <sys/time.h>
#include <climits>
#include <cstdlib>
#include <cstring>
#include "MeraDrpRuntimeWrapper.h"
#include "opencv2/core.hpp"
#include "iostream"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <vector>
#include <memory>
#include <chrono>
#include <cmath>
#include <queue>
#include <thread>
#include "PreRuntime.h"
#include <optional>
#include <utility> // for std::pair

#include "SparkProducerSocket.h"
#include "DiskUtils.h"
#include "ParkingSpot.h"

/* DRP-AI memory offset for model object file*/
#define DRPAI_MEM_OFFSET (0X38E0000)

using namespace cv;
using namespace std;

namespace
{
    const char *splash_screen = "spark_bg.png";
    const std::string model_dir = "parking_model";
    const std::string app_name = "SPARK";

    const std::string DRAG_MESSAGE = "Use left click+drag to select parking spaces";
    const std::string UNDO_MESSAGE = "Use right click to delete most recent parking space";

    const auto WHITE = cv::Scalar(255, 255, 255);
    const auto BLACK = cv::Scalar(0, 0, 0);

    const auto BUTTON_1_TL = cv::Point(415, 523);
    const auto BUTTON_1_BR = cv::Point(565, 573);
    const auto BUTTON_2_TL = cv::Point(687, 523);
    const auto BUTTON_2_BR = cv::Point(900, 573);

    // color-blind friendly palette
    const auto AVNET_GREEN = cv::Scalar(0, 206, 137);
    const auto AVNET_COMPLEMENTARY = cv::Scalar(138, 48, 230);
    const auto OCCUPIED_COLOR = cv::Scalar(AVNET_COMPLEMENTARY);
    const auto UNOCCUPIED_COLOR = cv::Scalar(AVNET_GREEN);

    const double NORMAL_FONT_SCALE = 1.0;
    const double PRIMARY_LABEL_SCALE = NORMAL_FONT_SCALE;
    const double SECONDARY_LABEL_SCALE = PRIMARY_LABEL_SCALE * 0.75;

    const int ESC_KEY = 27;

    const auto TRANSMISSION_PERIOD = std::chrono::seconds(2);

    void printMatInfo(const cv::Mat &mat)
    {
        // Print dimensions
        std::cout << "Dimensions: " << mat.cols << " x " << mat.rows;
        if (mat.channels() > 1)
        {
            std::cout << " x " << mat.channels();
        }
        std::cout << std::endl;

        // Calculate and print memory size
        size_t elementSize = mat.elemSize(); // Size of one matrix element in bytes
        size_t totalElements = mat.total();  // Total number of elements
        size_t totalSize = elementSize * totalElements;
        std::cout << "Estimated Memory Size: " << totalSize << " bytes" << std::endl;
    }

    float float16_to_float32(uint16_t a)
    {
        return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
    }

    /// @brief Display header1 and header2 text on img as white text on on the black background
    /// @param img img to write over
    /// @param header1 Primary header
    /// @param header2 Secondary header
    void display_header1_header2(Mat &img, const std::string &header1, const std::string &header2)
    {
        int thickness = 2;
        // out variables
        int baseline_drp = 0;
        int baseline_esc = 0;

        const auto drp_header_size = getTextSize(header1, FONT_HERSHEY_DUPLEX, NORMAL_FONT_SCALE, thickness, &baseline_drp);
        Point drp_header_org(0, 20);
        const auto esc_header_size = getTextSize(header2, FONT_HERSHEY_DUPLEX, SECONDARY_LABEL_SCALE, thickness, &baseline_esc);
        Point esc_header_org(0, 40);

        rectangle(img, drp_header_org + Point(0, baseline_drp), drp_header_org + Point(drp_header_size.width, -drp_header_size.height), BLACK, FILLED);
        rectangle(img, esc_header_org + Point(0, baseline_esc), esc_header_org + Point(esc_header_size.width, -esc_header_size.height), BLACK, FILLED);

        putText(img, header1, drp_header_org, FONT_HERSHEY_DUPLEX, NORMAL_FONT_SCALE, WHITE, 2);
        putText(img, header2, esc_header_org, FONT_HERSHEY_DUPLEX, SECONDARY_LABEL_SCALE, WHITE, 2);
    }
}

/* Global variables */
vector<ParkingSpot> parking_spots;

Mat img;
Mat frame1 = Mat::zeros(400, 400, CV_8UC3);

std::string filename;

Point2f box_start, box_end;
cv::Rect rect;
bool add_slot_in_figure = false;
bool start_inference_parking_slot = false;
bool drawing_box = false;
bool re_draw = false;
bool camera_input = false;

MeraDrpRuntimeWrapper runtime;

bool runtime_status = false;

/**
 * Convert HWC format to CHW
 */
cv::Mat hwc2chw(const cv::Mat &image)
{
    std::vector<cv::Mat> rgb_images;
    cv::split(image, rgb_images);
    cv::Mat m_flat_r = rgb_images[0].reshape(1, 1);
    cv::Mat m_flat_g = rgb_images[1].reshape(1, 1);
    cv::Mat m_flat_b = rgb_images[2].reshape(1, 1);
    cv::Mat matArray[] = {m_flat_r, m_flat_g, m_flat_b};
    cv::Mat flat_image;
    cv::hconcat(matArray, 3, flat_image);
    return flat_image;
}

cv::Mat hwc2chwNormalized(const cv::Mat &image, const cv::Scalar &mean, const cv::Scalar &std)
{
    // Check if image is empty
    if (image.empty())
    {
        std::cerr << "Input image is empty." << std::endl;
        return cv::Mat();
    }

    // Convert image to floating point for processing
    cv::Mat imageFloat;
    image.convertTo(imageFloat, CV_32FC3, 1.0 / 255);

    // Split the image into separate color channels
    std::vector<cv::Mat> rgb_images;
    cv::split(imageFloat, rgb_images);

    // Normalize each channel
    for (int i = 0; i < 3; ++i)
    {
        rgb_images[i] = (rgb_images[i] - mean[i]) / std[i];
    }

    // Reshape each channel to a single row (flatten)
    cv::Mat m_flat_r = rgb_images[0].reshape(1, 1);
    cv::Mat m_flat_g = rgb_images[1].reshape(1, 1);
    cv::Mat m_flat_b = rgb_images[2].reshape(1, 1);

    // Concatenate the flattened channels into a single matrix
    cv::Mat flat_image;
    cv::vconcat(std::vector<cv::Mat>{m_flat_r, m_flat_g, m_flat_b}, flat_image);

    return flat_image;
}

/*****************************************
 * Function Name     : get_patches
 * Description       : Function for drawing bounding box for parking slot.
 * Arguments         : event = int number
 *                         x = int number
 *                         y = int number
 ******************************************/
void get_patches(int event, int x, int y, int flags, void *param)
{
    // clone the image so we can mutate parking_spots array without leaving artifacts on img
    cv::Mat frame_copy = img.clone();
    if (event == EVENT_LBUTTONDOWN)
    {
        drawing_box = true;
        box_start = Point2f(x, y);
    }
    else if (event == EVENT_MOUSEMOVE)
    {
        if (drawing_box)
            box_end = Point2f(x, y);
    }
    else if (event == EVENT_LBUTTONUP)
    {
        drawing_box = false;
        box_end = Point2f(x, y);
        auto new_rect = cv::Rect(box_start, box_end);

        if (!new_rect.empty())
        {
            parking_spots.push_back(ParkingSpot(parking_spots.size() + 1, new_rect));
            // putText(img, "id: " + to_string(parking_spots.size() + 1), box_start, FONT_HERSHEY_DUPLEX, 1.0, BLACK, 2);
        }
    }
    else if (event == EVENT_RBUTTONDOWN)
    {
        std::cout << "Right button clicked" << std::endl;
        if (!parking_spots.empty())
        {
            parking_spots.pop_back();
        }
    }

    // Draw in-progress box
    if (drawing_box)
    {
        rectangle(frame_copy, box_start, box_end, AVNET_COMPLEMENTARY, 2);
    }

    // Draw complete/official parking_spots
    display_header1_header2(frame_copy, DRAG_MESSAGE, UNDO_MESSAGE);
    for (const auto &parking_spot : parking_spots)
    {
        putText(frame_copy, "id: " + std::to_string(parking_spot.slot_id), parking_spot.coords.tl(), FONT_HERSHEY_DUPLEX, 1.0, AVNET_COMPLEMENTARY, 2);
        rectangle(frame_copy, parking_spot.coords, AVNET_COMPLEMENTARY, 2);
    }
    box_end = Point2f(x, y);
    imshow("Draw parking_spots with mouse, press <esc> to return to inference", frame_copy);
}
/*****************************************
 * Function Name     : draw_rectangle
 * Description       : Function for initializing get patches.
 * Return value      : int = int number
 ******************************************/
int draw_rectangle(void)
{
    // Clone the image so we can mutate parking_spots array without leaving artifacts on img
    auto img_clone = img.clone();
    for (int i = 0; i < parking_spots.size(); i++)
    {
        rectangle(img_clone, parking_spots[i].coords, AVNET_COMPLEMENTARY, 2);
        putText(img_clone, "id: " + to_string(i + 1), Point(parking_spots[i].coords.x + 10, parking_spots[i].coords.y - 10), FONT_HERSHEY_DUPLEX, 1.0, AVNET_COMPLEMENTARY, 2);
    }
    display_header1_header2(img_clone, DRAG_MESSAGE, UNDO_MESSAGE);

    unsigned int key = 0;
    const std::string draw_rect_window = "Draw parking_spots with mouse, press <esc> to return to inference";
    cv::namedWindow(draw_rect_window, cv::WINDOW_NORMAL);
    setWindowProperty(draw_rect_window, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::imshow(draw_rect_window, img_clone);
    cv::setMouseCallback(draw_rect_window, get_patches, nullptr);
    key = cv::waitKey(0);
    if (key == 114) // Wait for 'r' key press to redraw!!
    {
        std::cout << "re-draw!!\n";
        cv::destroyAllWindows();
        return 1;
    }
    else
    {
        cv::destroyAllWindows();
        return 0;
    }
}

/*****************************************
 * Function Name     : addButtonCallback
 * Description       : add slots to the parking_spots vector(user can draw bounding box)
 ******************************************/
void addButtonCallback(int, void *)
{
    cv::VideoCapture vid;
redraw_rectangle:
    if (camera_input)
    {
        vid.open(0);
        vid.set(CAP_PROP_FRAME_WIDTH, 1920);
        vid.set(CAP_PROP_FRAME_HEIGHT, 1080);
    }
    else
        vid.open(filename);
    for (int frame = 0; frame < 10; frame++)
    {
        vid.read(img);
    }
    bool is_success = vid.read(img);
    if (is_success == true)
        std::cout << "Draw rectangle !!!\n";
    vid.release();
    re_draw = draw_rectangle();
    if (re_draw == true)
        goto redraw_rectangle;
}

/*****************************************
 * Function Name     : main_window_mouse_callback
 * Description       : Handles edit slot / start inference state transitions/forwarding
 ******************************************/
void main_window_mouse_callback(int event, int x, int y, int flags, void *userdata)
{
    auto edit_slots_rect = Rect(BUTTON_1_TL, BUTTON_1_BR);
    auto start_inference_rect = Rect(BUTTON_2_TL, BUTTON_2_BR);
    if (event == EVENT_LBUTTONDOWN)
    {
        if (edit_slots_rect.contains(Point(x, y)))
        {
            add_slot_in_figure = true;
        }
        else if (start_inference_rect.contains(Point(x, y)))
        {
            start_inference_parking_slot = true;
        }
    }
}

void read_frames(const string &videoFile, queue<Mat> &frames, bool &stop)
{
    VideoCapture cap;
    if (filename == "0")
    {
        cap.open(0);
        cap.set(CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(CAP_PROP_FRAME_HEIGHT, 1080);
    }
    else
    {
        cap.open(videoFile);
    }

    if (!cap.isOpened())
    {
        cerr << "Failed " << videoFile << endl;
        return;
    }

    try
    {
        Mat frame;
        while (!stop)
        {
            if (!cap.read(frame) || frame.empty())
                throw new runtime_error("Failed to read frame from video file");

            frames.push(frame);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;

        stop = true;
        cap.release();
        return;
    }
}

/// @brief The primary business logic of the parking lot detection application
/// @param frames The queue of VideoCapture frames to process
/// @param stop The flag to stop the processing
/// @param producerSocket The SparkProducerSocket to send occupancy data through
void process_frames(queue<Mat> &frames, bool &stop, std::shared_ptr<SparkProducerSocket> producerSocket)
{

    Rect box;
    Mat patch1, patch_con, patch_norm, inp_img;
    namedWindow(app_name, WINDOW_NORMAL);
    setWindowProperty(app_name, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    if (!parking_spots.empty())
    {
        disk_utils::serializeROIs(parking_spots);
    }

    while (!stop)
    {
        if (!frames.empty())
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            Mat frame = frames.front();
            frames.pop();
            img = frame;
            int taken = 0, empty = 0;
            for (auto &parking_spot : parking_spots)
            {
                box = parking_spot.coords;
                patch1 = img(box);
                resize(patch1, patch1, Size(28, 28));
                // patch is 28x28x3 (aka dont forget its BGR)
                cvtColor(patch1, patch1, COLOR_BGR2RGB);
                inp_img = hwc2chw(patch1);

                if (!inp_img.isContinuous())
                    patch_con = inp_img.clone();
                else
                    patch_con = inp_img;

                // Replicate the 'ToTensor()' function in the PyTorch model
                patch_con.convertTo(patch_norm, CV_32F, 1.0 / 255.0, 0);
                runtime.SetInput(0, patch_norm.ptr<float>());
                runtime.Run();
                auto output_num = runtime.GetNumOutput();
                if (output_num != 1)
                {
                    std::cerr << "[ERROR] Output size : not 1." << std::endl;
                    return;
                }
                auto output_buffer = runtime.GetOutput(0);
                int64_t out_size = std::get<2>(output_buffer);
                float floatarr[out_size];

                if (InOutDataType::FLOAT16 == std::get<0>(output_buffer))
                {
                    /* Extract data in FP16 <uint16_t>. */
                    uint16_t *data_ptr = reinterpret_cast<uint16_t *>(std::get<1>(output_buffer));

                    /* Post-processing for FP16 */
                    /* Cast FP16 output data to FP32. */
                    for (int n = 0; n < out_size; n++)
                    {
                        floatarr[n] = float16_to_float32(data_ptr[n]);
                    }
                }
                else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer))
                {
                    /* Extract data in FP32 <float>. */
                    float *data_ptr = reinterpret_cast<float *>(std::get<1>(output_buffer));
                    /*Copy output data to buffer for post-processing. */
                    std::copy(data_ptr, data_ptr + out_size, floatarr);
                }
                else
                {
                    std::cerr << "[ERROR] Output data type : not floating point type." << std::endl;
                    return;
                }

                std::string label;
                Scalar boxColor;
                const bool is_occupied = floatarr[0] < floatarr[1];
                if (is_occupied)
                {
                    taken++;
                    label = "taken";
                    boxColor = OCCUPIED_COLOR;
                }
                else
                {
                    empty++;
                    label = "empty";
                    boxColor = UNOCCUPIED_COLOR;
                }
                parking_spot.update_occupancy(is_occupied);

                int baseline = 0;
                int thickness = 2;
                Size textSize = getTextSize("id: " + to_string(parking_spot.slot_id), FONT_HERSHEY_DUPLEX, SECONDARY_LABEL_SCALE, thickness, &baseline);
                Size labelSize = getTextSize(label, FONT_HERSHEY_DUPLEX, PRIMARY_LABEL_SCALE, thickness, &baseline);

                // Calculate the position for the text background
                Point textOrg(parking_spot.coords.x + parking_spot.coords.width - textSize.width - thickness, parking_spot.coords.y + parking_spot.coords.height - 5 * thickness);
                Point labelOrg(parking_spot.coords.x, parking_spot.coords.y - baseline - thickness);

                // Draw the background rectangle for better visibility
                rectangle(img, textOrg + Point(0, baseline), textOrg + Point(textSize.width, -textSize.height), boxColor, FILLED);
                rectangle(img, labelOrg + Point(0, baseline), labelOrg + Point(labelSize.width, -labelSize.height), boxColor, FILLED);

                // Now draw the text over the rectangle
                putText(img, "id: " + to_string(parking_spot.slot_id), textOrg + Point(0, 3), FONT_HERSHEY_DUPLEX, SECONDARY_LABEL_SCALE, BLACK, thickness);
                putText(img, label, labelOrg + Point(0, 2), FONT_HERSHEY_DUPLEX, PRIMARY_LABEL_SCALE, BLACK, thickness);

                rectangle(img, parking_spot.coords, boxColor, thickness);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

            const std::string drp_header = "DRP-AI Processing Time: " + to_string(duration) + " ms";
            const std::string esc_header = "Press esc to go back";
            display_header1_header2(img, drp_header, esc_header);

            if (waitKey(3) == ESC_KEY) // Wait for 'Esc' key press to stop inference window!!
            {
                stop = true;
                for (auto &spot : parking_spots)
                {
                    spot.is_online = false;
                }

                destroyAllWindows();
                break;
            }

            imshow(app_name, img);
            if (producerSocket && producerSocket->sendOccupancyDataThrottled(parking_spots))
            {
                // std::cout << "Sent occupancy data" << std::endl;
            }
        }
    }
}

/*****************************************
 * Function Name : get_drpai_start_addr
 * Description   : Function to get the start address of DRPAImem.
 * Arguments     : -
 * Return value  : uint32_t = DRPAImem start address in 32-bit.
 ******************************************/
std::optional<uint32_t> get_drpai_start_addr()
{
    int fd = 0;
    int ret = 0;
    // Defined in drpai.h
    drpai_data_t drpai_data;

    errno = 0;

    fd = open("/dev/drpai0", O_RDWR);
    if (0 > fd)
    {
        LOG(FATAL) << "[ERROR] Failed to open DRP-AI Driver : errno=" << errno;
        return std::nullopt;
    }

    /* Get DRP-AI Memory Area Address via DRP-AI Driver */
    ret = ioctl(fd, DRPAI_GET_DRPAI_AREA, &drpai_data);
    if (-1 == ret)
    {
        LOG(FATAL) << "[ERROR] Failed to get DRP-AI Memory Area : errno=" << errno;
        return std::nullopt;
    }

    return drpai_data.address;
}

int main(int argc, char **argv)
{

    /*Load model_dir structure and its weight to runtime object */
    auto drpaimem_addr_start = get_drpai_start_addr();
    if (!drpaimem_addr_start.has_value())
    {
        /* Error notifications are output from function get_drpai_start_addr(). */
        fprintf(stderr, "[ERROR] Failed to get DRP-AI memory area start address. \n");
        return -1;
    }

    parking_spots = disk_utils::deserializeROIs();

    std::shared_ptr<SparkProducerSocket> producerSocket;
    try
    {
        producerSocket = std::make_shared<SparkProducerSocket>("::1", 50000);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        producerSocket = nullptr;
    }

    // runtime_status = false
    runtime_status = runtime.LoadModel(model_dir, drpaimem_addr_start.value() + DRPAI_MEM_OFFSET);

    if (!runtime_status)
    {
        fprintf(stderr, "[ERROR] Failed to load model. \n");
        return -1;
    }

    std::cout << "loaded model:" << model_dir << "\n";

    if (argc == 1)
    {
        std::cout << "Loading from camera input...\n";
        camera_input = true;
        filename = "0";
    }
    else
    {
        filename = argv[1];
        camera_input = false;
        std::cout << "Loading from :" << filename << "\n";
    }
    namedWindow(app_name, WINDOW_NORMAL);
    resizeWindow(app_name, 1200, 800);
    int key = -1;
    while ((key = waitKey(1)) != 'q' && key != ESC_KEY)
    {
        Mat frame;
        frame = cv::imread(splash_screen);
        cv::resize(frame, frame, cv::Size(1200, 800));
        if (add_slot_in_figure)
        {
            add_slot_in_figure = false;
            addButtonCallback(0, 0);
            waitKey(0);
        }
        else
        {
            rectangle(frame, Point(415, 523), Point(565, 573), BLACK, -1);
            putText(frame, "Edit Slots", Point(415 + (int)150 / 4, 553), FONT_HERSHEY_SIMPLEX, 0.5, WHITE, 1);
        }

        if (start_inference_parking_slot)
        {
            start_inference_parking_slot = false;

            destroyAllWindows();
            std::cout << "Running TVM runtime" << std::endl;

            queue<Mat> frames;
            bool stop = false;
            thread readThread(read_frames, filename, ref(frames), ref(stop));
            cout << "Waiting for read frames to add frames to buffer!" << endl;
            this_thread::sleep_for(std::chrono::seconds(0));
            thread processThread(process_frames, ref(frames), ref(stop), producerSocket);
            cout << "Processing thread started......" << endl;
            waitKey(0);
            stop = false;
            readThread.join();
            processThread.join();
        }
        else
        {
            if (!parking_spots.empty())
            {
                const std::string slot_text = "Monitoring " + std::to_string(parking_spots.size()) + " slots";
                int baseline_slot_text = 0;
                const auto slot_text_size = getTextSize(slot_text, FONT_HERSHEY_SIMPLEX, SECONDARY_LABEL_SCALE, 2, &baseline_slot_text);
                putText(frame, slot_text, BUTTON_2_TL - Point(0, baseline_slot_text), FONT_HERSHEY_SIMPLEX, SECONDARY_LABEL_SCALE, WHITE, 2);
            }
            rectangle(frame, BUTTON_2_TL, BUTTON_2_BR, BLACK, -1);
            putText(frame, "Start Inference", Point(687 + (int)150 / 4, 553), FONT_HERSHEY_SIMPLEX, 0.5, WHITE, 1);
        }
        setMouseCallback(app_name, main_window_mouse_callback, &add_slot_in_figure);
        imshow(app_name, frame);
    }
    return 0;
}
