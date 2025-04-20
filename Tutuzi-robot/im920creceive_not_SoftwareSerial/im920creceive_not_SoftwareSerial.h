void valprint();
void ascii_to_hex(char *, char *);
int receive_timecount = 0;

void im920creceive_setup() {  //メインファイルのsetup()にこの関数を入れる
  Serial3.begin(19200);
}

void DUE_im920creceive(char *returndata, bool *disconnect_p) {
  static char DUE_im920creceive_input[40];  //staticなので、メインプログラムで変数名が重複しないように、あえて長い変数名にしてある
  char inputsub;
  static int DUE_im920creceive_counter = 0;  //staticなので、メインプログラムで変数名が重複しないように、あえて長い変数名にしてある


  while (Serial3.available()) {
    inputsub = Serial3.read();
    DUE_im920creceive_input[DUE_im920creceive_counter] = inputsub;
    DUE_im920creceive_counter++;
    if (inputsub == '\n') {
      DUE_im920creceive_counter = 0;
      ascii_to_hex(DUE_im920creceive_input, returndata);
      receive_timecount = 0;
      *disconnect_p = 0;
    }
  }
}

void ascii_to_hex(char *im920creceive, char *outdata) {
  char input_data[16];
  int j;
  for (j = 0; j < 40; j++) {
    if (im920creceive[j] == ':') {
      input_data[0] = im920creceive[j + 1];
      input_data[1] = im920creceive[j + 2];
      input_data[2] = im920creceive[j + 4];
      input_data[3] = im920creceive[j + 5];
      input_data[4] = im920creceive[j + 7];
      input_data[5] = im920creceive[j + 8];
      input_data[6] = im920creceive[j + 10];
      input_data[7] = im920creceive[j + 11];
      input_data[8] = im920creceive[j + 13];
      input_data[9] = im920creceive[j + 14];
      input_data[10] = im920creceive[j + 16];
      input_data[11] = im920creceive[j + 17];
      input_data[12] = im920creceive[j + 19];
      input_data[13] = im920creceive[j + 20];
      input_data[14] = im920creceive[j + 22];
      input_data[15] = im920creceive[j + 23];
      break;
    }
  }
  for (j = 0; j < 16; j++) {
    switch (input_data[j]) {
      case '0': input_data[j] = 0; break;
      case '1': input_data[j] = 1; break;
      case '2': input_data[j] = 2; break;
      case '3': input_data[j] = 3; break;
      case '4': input_data[j] = 4; break;
      case '5': input_data[j] = 5; break;
      case '6': input_data[j] = 6; break;
      case '7': input_data[j] = 7; break;
      case '8': input_data[j] = 8; break;
      case '9': input_data[j] = 9; break;
      case 'A': input_data[j] = 10; break;
      case 'B': input_data[j] = 11; break;
      case 'C': input_data[j] = 12; break;
      case 'D': input_data[j] = 13; break;
      case 'E': input_data[j] = 14; break;
      case 'F': input_data[j] = 15; break;
    }
  }
  for (j = 0; j < 8; j++) {
    outdata[j] = (input_data[j * 2] << 4) | (input_data[(j * 2) + 1]);
  }

  /*for (int k = 0; k < 8; k++) {
    Serial3.print((int)outdata[k]);
    Serial3.print(' ');
  }
  Serial3.print("\r\n");*/
}
