#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include<sys/timeb.h>

//C++�е�sstream�࣬�ṩ�˳����string����֮���I/O������ͨ��
//ostringstream��istringstream���������������󣬷ֱ��Ӧ�������������
#include "kcftracker.hpp"
#include "TLD.h"
#include "tld_utils.h"
using namespace cv;
using namespace std;
//Global variables
//ȫ�ֱ���
Rect box;
bool drawing_box = false;
bool gotBB = false;
bool tl = true;
bool rep = false;
bool fromfile=false;
string video;


//�������ܣ���txt�ļ��ж�ȡbox���ݵ�ȫ�ֱ���box��
/**********************************************************
	��ȡ��¼bounding box���ļ������bounding box���ĸ����������Ͻ�
	���꣨x��y���Ϳ��
***********************************************************/
void readBB(char* file){
  ifstream bb_file (file);   //�����ļ�������
  string line;
  getline(bb_file,line); //��ȡ�ļ��еĵ�һ��
						 //��������bb_file�ж������ַ�����string
						 //����line�У��ս��Ĭ��Ϊ'\n'
  istringstream linestream(line); //istringstream������԰�һ��
								  //�ַ�����Ȼ���Կո�Ϊ�ָ����Ѹ���
								  //�ָ�����
  string x1,y1,x2,y2;
  getline (linestream,x1, ','); //��������linestream�������ַ�����
  getline (linestream,y1, ','); //�����x1,y1,x2,y2�ַ���������
  getline (linestream,x2, ','); //ֱ�������ս����������ֹ��ȡ��
  getline (linestream,y2, ',');
  int x = atoi(x1.c_str());//���ַ���ת����������
  int y = atoi(y1.c_str());// = (int)file["bb_y"];
  int w = atoi(x2.c_str())-x;// = (int)file["bb_w"];
  int h = atoi(y2.c_str())-y;// = (int)file["bb_h"];
  box = Rect(x,y,w,h);
}
string Int_to_String(int n)
{
	ostringstream stream;
	stream << n;  //nΪint����
	return stream.str();
}
/****************************************************
   bounding box mouse callback
   ������¼���Ӧ������������Ϊ�趨�ĳ�ʼbox
*****************************************************/
void mouseHandler(int event, int x, int y, int flags, void *param){
  switch( event ){
  case CV_EVENT_MOUSEMOVE:
    if (drawing_box){
        box.width = x-box.x;
        box.height = y-box.y;
    }
    break;
  case CV_EVENT_LBUTTONDOWN:
    drawing_box = true;
    box = Rect( x, y, 0, 0 );
    break;
  case CV_EVENT_LBUTTONUP:
    drawing_box = false;
    if( box.width < 0 ){
        box.x += box.width;
        box.width *= -1;
    }
    if( box.height < 0 ){
        box.y += box.height;
        box.height *= -1;
    }
    gotBB = true;  //�Ѿ����bounding box
    break;
  }
}

void print_help(char** argv){
  printf("use:\n     %s -p /path/parameters.yml\n",argv[0]);
  printf("-s    source video\n-b        bounding box file\n-tl  track and learn\n-r     repeat\n");
}

int main(int argc, char * argv[])
{
  VideoCapture capture(0);
  FileStorage fs; //�洢ϵͳ�õ��Ĳ�������parameters.yml���
  fstream processfream;
  processfream.open("processfream_time.txt",ios::app);
//  read_options(argc,argv,capture,fs);  //��ȡ����
  fs.open("parameters.yml", FileStorage::READ);
 
 
  cvNamedWindow("TLD",CV_WINDOW_AUTOSIZE);
  if(!gotBB)
  {
	  cvSetMouseCallback( "TLD", mouseHandler, NULL );
  }


  //TLD framework
  TLD tld;
  //Read parameters file
  tld.read(fs.getFirstTopLevelNode());
  Mat frame;
  Mat last_gray;
  Mat first;
  if (fromfile){ //���ָ��Ϊ���ļ��ж�ȡ
	  capture >> frame; //��ȡ��Ƶ֡
      cvtColor(frame, last_gray, CV_RGB2GRAY); //ת��Ϊ�Ҷ�ͼ��
      frame.copyTo(first);  //������Ϊ��һ֡
  }
  else{
	  capture.open("C://Users//Administrator//Desktop//����ѧϰ������//datasets//panda.mpg");  //"C://Users//Administrator//Desktop//����ѧϰ������//datasets//pedestrian2.mpg"
	  if (!capture.isOpened())  //�ж�����ͷ�Ƿ��
	  {
			cout << "capture device failed to open!" << endl;
			return 1;
	  }

//      capture.set(CV_CAP_PROP_FRAME_WIDTH,340); //���û�ȡ��ͼ���СΪ320*240
//      capture.set(CV_CAP_PROP_FRAME_HEIGHT,240);
  }
  
  ///Initialization
GETBOUNDINGBOX: //��ţ���ȡbounding box
  while(!gotBB)
  {
    if (!fromfile){
      capture >> frame;
	  
    }
    else
      first.copyTo(frame);
    cvtColor(frame, last_gray, CV_RGB2GRAY);
	drawBox(frame, box, Scalar(0, 255, 0));  //��boundbing box ������
    imshow("TLD", frame);

	int delay = 30;
	if (delay >= 0 && waitKey(delay) >= 0)//�ո������ͣ��Ƶ
		waitKey(0);

    if (cvWaitKey(33) == 'q')
	    return 0;
  }
  //����ͼ��Ƭ��min_winΪ15*15���أ�����bounding box�в����õ��ġ�����box�����
  //min_winҪ��
  if (min(box.width,box.height)<(int)fs.getFirstTopLevelNode()["min_win"])
  {
      cout << "Bounding box too small, try again." << endl;
      gotBB = false;
      goto GETBOUNDINGBOX;
  }
  //Remove callback
  cvSetMouseCallback( "TLD", NULL, NULL );//����Ѿ���õ�һ֡�û��򶨵�box�˾�ȡ��
										  //�����Ӧ
  printf("Initial Bounding Box = x:%d y:%d h:%d w:%d\n",box.x,box.y,box.width,box.height);
  //Output file
  FILE  *bb_file = fopen("bounding_boxes.txt","w");


  //TLD initialization
  //TLD ���г�ʼ��
  tld.init(last_gray,box,bb_file);

  ///Run-time
  Mat current_gray;
  BoundingBox pbox;
  vector<Point2f> pts1;
  vector<Point2f> pts2;
  bool status=true;  //��¼���ٳɹ�����״̬ lastbox been found
  int frames = 1;    //��¼�ѹ�ȥ��֡��
  int detections = 1; //��¼�ɹ���⵽��Ŀ��box��Ŀ
REPEAT:
  while(capture.read(frame)){

    //get frame
    cvtColor(frame, current_gray, CV_RGB2GRAY);
    //Process Frame
	struct timeb start, end;
	ftime(&start);

	tld.processFrame(last_gray, current_gray, pbox, status, tl, bb_file, frame, box);

	ftime(&end);
	std::cout << "processframe Process Time: " << (end.time - start.time) * 1000 + (end.millitm - start.millitm) << " ms" << endl;

	processfream << "frames : " << frames << "processframe Process Time: " << (end.time - start.time) * 1000 + (end.millitm - start.millitm) << "ms" << endl;

    //Draw Points
    if (status){ //������ٳɹ�
      drawPoints(frame,pts1);
      drawPoints(frame,pts2,Scalar(0,255,0)); //��ǰ������������ɫ���ʾ
      drawBox(frame,pbox,Scalar(0,255,0));
      detections++;
    }
	//output bounding box
	
    //Display
	char strFrame[20];
	sprintf(strFrame, "#%d ",frames) ;
	putText(frame,strFrame,cvPoint(0,20),2,1,CV_RGB(25,200,25));
    imshow("TLD", frame);
    //swap points and images
    swap(last_gray,current_gray); //STL����swap()���������������ֵ���䷺�ͻ��汾
								  //������<algorithm>
    pts1.clear();
    pts2.clear();
    frames++;
    printf("Detection rate: %d/%d\n",detections,frames);

	if (status)
	{
		FileStorage boundingbox("newboundingbox.txt", FileStorage::APPEND);
		boundingbox << "framenumber" << frames << "x" << pbox.x << "y" << pbox.y << "width" << pbox.width << "height" << pbox.height;
	}

	//string doc("volkswagen");
	//int frameNum = 1;

	//imwrite(".//" + doc + "//volkswagenframe" + Int_to_String(frames) + ".jpg", frame);//���浽��ǰ�ļ����µ�cars6�ļ�����

    if (cvWaitKey(33) == 'q')
      break;
  }
  if (rep){
    rep = false;
    tl = false;
    fclose(bb_file);
    bb_file = fopen("final_detector.txt","w");
    //capture.set(CV_CAP_PROP_POS_AVI_RATIO,0);
    capture.release();
    capture.open(video);
    goto REPEAT;
  }
  fs.release();
  fclose(bb_file);
  processfream.close();
  return 0;
}

