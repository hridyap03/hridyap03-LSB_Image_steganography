Name:Hridya P
Date:15/12/2025
Description:Steganography
Input:./a.out -e beautiful.bmp secret.txt stego.bmp
Output:
    -------------Selected mode is encoding-------------
    Opened files successfully
    width = 1024
    height = 768
    Check capacity successful
    Copied bmp header successfully
    Encoded magic string size successfully
    Encoded magic string successfully
    Encoded size of secret file extension successfully
    Encoded secret file extension successfully
    Encoded secret file size successfully
    Encoded secret file successfully
    Encoded remaining image data successfully
    -------------Encoding successful-------------
Input:./a.out -d stego.bmp output.txt
Output:
    -------------Selected mode is decoding-------------
    Skipped bmp header successfully
    Decoded magic string size successfully
    Enter the magic string : #* 
    Decoded magic string successfully
    Decoded secret file extention size successfully
    Decoded secret file extention successfully
    Decoded secret file size successfully
    Decoded secret data successfully
    -------------Decoding successful-------------
