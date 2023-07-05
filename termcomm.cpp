/**********************************************************
 *
 * Программа демонстрации псевдографики в консоли на C
 * с одновременным чтениеь компорта
 * и записью результата в окна консоли (в отдельном потоке)
 *
 * Данные получает из порта, куда они поступают от Arduino
 * Arduino имеет на борту несколько тест-генераторов
 * один из которых (4-й) выдает 5 троек чисел float
 *
 * Через компорт поступает строка с числами в формате
 * float, разделенных пробелами и символом конца строки в конце
 * Частота прихода таких строк ограничена baudrate == 19200
 * в данном случае, а частота генерации строки паузой 100 мс,
 * но были испытаны и другие паузы вплоть до 10 мс  
 *
 * Author:  Starmark LN (Kornilov LN)
 * e-mail:  starmark.ln@ekatra.io
 *          starmark.ln@gmail.com
 * date:    21.06.2023
 *
 *********************************************************/

#include "termcomm.h"
#include "comm.h"

#include <stdexcept>
#include <thread>
#include <mutex>

#define COLUMNAMES " Port A     Signal{xyz}   MEMS{AGM} "

std::mutex cout_mutex;

stArrPayval dt;


stFrameXY   frmTitle= {2,2};
stFrameXY   frmColNam = {2,4};
stFrameXY   frmBits = {2,6};
stFrameXY   frmClim = {13,6};
stFrameXY   frmDtim = {13,11};
stFrameXY   frmBme  = {13,11};
stFrameXY   frmAccl = {27,6};
stFrameXY   frmGiro = {27,10};
stFrameXY   frmMagn = {27,14};
stFrameXY   frmAlrm = {2,16};


void frame_draw ();
void print_columnames (const char* cn);
void print_time_date (struct tm* tm_info);
unsigned char rol(unsigned char  a);
void print_port_bits (unsigned char port);
void print_alarm (int alarm_error);
void print_pay(payval& pv);
void print_payACC(payval& pv);
void print_payGIR(payval& pv);
void print_payMAG(payval& pv);
void print_payBME(payval& pv);

void print_Data(payval& pv, stFrameXY& frm, const char * name, const char * coordinate);

void controller ();

//=======================================================================

void controller () {
	unsigned char  port = 0x01;

	int alarm_counter = 0;
	time_t timer;
	struct tm* tm_info;
	srand(time(NULL));

    frame_draw ();
 
	while(1){
/*
		float a[3] = {0.0, 0.0, 0.0};

		for (int i=0; i<3; i++) {
			int r = rand() % 101 + (-50);
			a[i] += (float)r / 1000;
		}
*/
		if (alarm_counter < 30) {
			print_alarm(0);
			print_port_bits(port);
		}
		if ((alarm_counter < 60) && (alarm_counter >= 30)) {
			print_alarm(1);
		}
		if (alarm_counter > 60) {
			alarm_counter = 0;
		}
		alarm_counter++;

    	//--- Взять дату и время и подготовить к выводу
		time(&timer);
		tm_info = localtime(&timer);

    	//==================================
		print_Data(dt.pv,  frmClim, "fff", "012");
		print_Data(dt.Acc, frmAccl, "AAA", "xyz");
		print_Data(dt.Gir, frmGiro, "GGG", "xyz");
		print_Data(dt.Mag, frmMagn, "MMM", "xyz");
		print_Data(dt.Bme, frmBme,  "GGG", "xyz");
		print_time_date(tm_info);
    	//==================================

		port = rol(port);
		gotoxy(1,18);

		fflush(stdout);
        usleep(PAUSE_LEN);
    }
}

//=======================================================================

int main (void) {

	clPort prt;
    prt.Open();
	prt.SetAttributes();

    std::thread t1(controller);

    prt.Work(dt);

	prt.Close();
    t1.join();

	return 0;
}

//=======================================================================

void frame_draw (void) {
	home();
	clrscr();

	set_display_atrib(DISPLAY_BACKGROUND);
    puts(WIN);

    print_columnames (COLUMNAMES);
 
	resetcolor();
}

void print_alarm (int alarm_error) {
	gotoxy(frmAlrm.X,frmAlrm.Y);
	if (alarm_error) {
		set_display_atrib(DISPLAY_BRIGHT);
		set_display_atrib(B_BLUE);
        set_display_atrib(F_RED);
		printf("      Work ERROR       ");
	} else {
		set_display_atrib(DISPLAY_BRIGHT);
		set_display_atrib(B_GREEN);
		printf("        Work OK        ");
	}
}

unsigned char rol(unsigned char  a)
{
	return (a << 1) | (a >> 7);
}

void print_port_bits (unsigned char port) {	
	unsigned char maxPow = 1<<(8-1);
	set_display_atrib(DISPLAY_BACKGROUND);
	set_display_atrib(DISPLAY_BRIGHT);
	for(int i=0;i<8;++i){
		// print last bit and shift left.
		gotoxy(frmBits.X,frmBits.Y + i);
		if (port & maxPow) {
			set_display_atrib(F_YELLOW);
			printf("pin%d= on ",i);
		} else {
			set_display_atrib(F_CYAN);
			printf("pin%d= off",i);
		}
		port = port<<1;
	}

	resetcolor();
}

void print_columnames (const char* cn) {
	set_display_atrib(DISPLAY_BRIGHT);
	set_display_atrib(DISPLAY_BACKGROUND);
    set_display_atrib(F_WHITE);

	gotoxy(frmColNam.X,frmColNam.Y)
	puts(cn);

	resetcolor();
}

void print_time_date (struct tm* tm_info) {
	char buffer[27];
	set_display_atrib(DISPLAY_BRIGHT);
	set_display_atrib(DISPLAY_BACKGROUND);

	strftime(buffer, 27, "  Informator:   %d.%m.%y", tm_info);
	gotoxy(frmTitle.X,frmTitle.Y)
	puts(buffer);

	strftime(buffer, 10, "%H:%M:%S", tm_info);
	gotoxy(frmTitle.X+26,frmTitle.Y)
	puts(buffer);

	resetcolor();
}

void print_Data(payval& pv, stFrameXY& frm, const char * name, const char * coordinate) {
    char buf[16] = {'\0'};

	SETDISPLAYATTRIBS

	set_display_atrib(display_atrib + 0);
	gotoxy(frm.X,frm.Y+0);
    sprintf(buf, "%c%c = %+3.2f%c", name[0], coordinate[0], pv.f0, 0);
    puts(buf);

	set_display_atrib(display_atrib + 1);
	gotoxy(frm.X,frm.Y+1);
	sprintf(buf, "%c%c = %+3.2f%c", name[1], coordinate[1], pv.f1, 0);
    puts(buf);

	set_display_atrib(display_atrib + 2);
	gotoxy(frm.X,frm.Y+2);
	sprintf(buf, "%c%c = %+3.2f%c", name[2], coordinate[2], pv.f2, 0);
    puts(buf);

	resetcolor();
}

//=======================================================================

/*
void frame_draw (void) {
	home();
	clrscr();
	set_display_atrib(DISPLAY_BACKGROUND);

//--- ANSI/VT100 Terminal----------------------------------
//           012345678901234567890123456789012345678
	puts(	"┌─────────────────────────────────────┐\n" //0
			"│                                     │\n" //1
			"├─────────────────────────────────────┤\n" //2
			"│                                     │\n" //3
	     	"├─────────┐┌────────────┐┌────────────┤\n" //4
			"│         ││            ││            │\n" //5
			"│         ││            ││            │\n" //6
			"│         ││            ││            │\n" //7
			"│         │└────────────┘│            │\n" //8
			"│         │┌────────────┐│            │\n" //9
			"│         ││            ││            │\n" //0
			"│         ││            ││            │\n" //1
			"│         ││            ││            │\n" //2
			"└─────────┘└────────────┘│            │\n" //3
			"┌───────────────────────┐│            │\n" //4
			"│                       ││            │\n" //5
			"└───────────────────────┘└────────────┘"); //6

    print_columnames (COLUMNAMES);
 
	resetcolor();
}
*/

/*
void print_pay(payval& pv) {
	const char * coordinate = "012";

	SETDISPLAYATTRIBS

	set_display_atrib(display_atrib + 0);
	gotoxy(frmClim.X,frmClim.Y+0);
	printf("f%c = %+3.2f", coordinate[0], pv.f0);

	set_display_atrib(display_atrib + 1);
	gotoxy(frmClim.X,frmClim.Y+1);
	printf("f%c = %+3.2f", coordinate[1], pv.f1);

	set_display_atrib(display_atrib + 2);
	gotoxy(frmClim.X,frmClim.Y+2);
	printf("f%c = %+3.2f", coordinate[2], pv.f2);

	resetcolor();		
}

void print_payACC(payval& pv) {
	const char * coordinate = "xyz";

	SETDISPLAYATTRIBS

	set_display_atrib(display_atrib + 0);
	gotoxy(frmAccl.X,frmAccl.Y+0);
	printf("A%c = %+3.2f", coordinate[0], pv.f0);

	set_display_atrib(display_atrib + 1);
	gotoxy(frmAccl.X,frmAccl.Y+1);
	printf("A%c = %+3.2f", coordinate[1], pv.f1);

	set_display_atrib(display_atrib + 2);
	gotoxy(frmAccl.X,frmAccl.Y+2);
	printf("A%c = %+3.2f", coordinate[2], pv.f2);

	resetcolor();
}

void print_payGIR(payval& pv) {
	const char * coordinate = "xyz";

	SETDISPLAYATTRIBS

	set_display_atrib(display_atrib + 0);
	gotoxy(frmGiro.X,frmGiro.Y+0);
	printf("G%c = %+3.2f", coordinate[0], pv.f0);

	set_display_atrib(display_atrib + 1);
	gotoxy(frmGiro.X,frmGiro.Y+1);
	printf("G%c = %+3.2f", coordinate[1], pv.f1);

	set_display_atrib(display_atrib + 2);
	gotoxy(frmGiro.X,frmGiro.Y+2);
	printf("G%c = %+3.2f", coordinate[2], pv.f2);

	resetcolor();
}

void print_payMAG(payval& pv) {
	const char * coordinate = "xyz";

	SETDISPLAYATTRIBS

	set_display_atrib(display_atrib + 0);
	gotoxy(frmMagn.X,frmMagn.Y+0);
	printf("M%c = %+3.2f", coordinate[0], pv.f0);

	set_display_atrib(display_atrib + 1);
	gotoxy(frmMagn.X,frmMagn.Y+1);
	printf("M%c = %+3.2f", coordinate[1], pv.f1);

	set_display_atrib(display_atrib + 2);
	gotoxy(frmMagn.X,frmMagn.Y+2);
	printf("M%c = %+3.2f", coordinate[2], pv.f2);

	resetcolor();
}

void print_payBME(payval& pv) {
	const char * coordinate = "TPH";

	SETDISPLAYATTRIBS

	set_display_atrib(display_atrib + 0);
	gotoxy(frmBme.X,frmBme.Y+0);
	printf(" %c = %+3.2f", coordinate[0], pv.f0);

	set_display_atrib(display_atrib + 1);
	gotoxy(frmBme.X,frmBme.Y+1);
	printf(" %c = %+3.2f", coordinate[1], pv.f1);

	set_display_atrib(display_atrib + 2);
	gotoxy(frmBme.X,frmBme.Y+2);
	printf(" %c = %+3.2f", coordinate[2], pv.f2);

	resetcolor();
}
*/


