# INF2004-Project-P3A
**Robotic Car Project - Team P3A**

### 'Multicore' Branch Folder Structure
Template code for running freertos in 'Symmetric Multiprocessing' mode

    Main Folder/
    ├── CMakeLists.txt      # Root CMakeLists.txt (No need to touch)
    ├── main.c              # Main driver code goes here (Call all your functions in here)
    ├── FreeRTOS-Kernel/    # FreeRTOS kernel directory (No need to touch)
    ├── source/             # Source folder (Put all your codes here)
        └── CMakeLists.txt  # Update this with whatever new .c file you added
        └── header.h        # Update this with your function prototypes before calling in main.c
        └── blink.c         # Simple code to test that everything is working (No need to touch)