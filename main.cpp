#include <Windows.h>
#include <ctime>
#include <vector>
#include <cmath>
#include <iostream>
#include <io.h>
#include <fcntl.h>

#include <handle.hpp>
#include <shwin.hpp>

#define WW 512
#define HH 512

handle hd;
shwin wnd;

unsigned long comp_mspf() {
	static unsigned long old_time = clock()-15, new_time = clock(), dt = 15;
	new_time = clock();
	dt += new_time - old_time;
	dt >>= 1;
	old_time = new_time;
	return dt;
}

typedef struct imm {
    float x, y;
    imm(float _x = 0, float _y = 0) {
        x = _x; y = _y;
    }
    imm operator=(imm c) {
        x = c.x; y = c.y;
        return *this;
    }
    imm operator+(imm c) {
        return imm(x + c.x, y + c.y);
    }
    imm operator-() {
        return imm(-x, -y);
    }
    imm operator*(float c) {
        return imm(x * c, y * c);
    }
    imm operator/(float c) {
        return imm(x / c, y / c);
    }
    imm operator-(imm c) {
        return imm(x - c.x, y - c.y);
    }
    float length2() {
        return x*x + y*y;
    }
    float length() {
        return sqrt(x*x + y*y);
    }
} imm;

#define ll 500
#define gg 5

#define GSM 2000000.0f
float ae = 14;
#define am (0.39f*ae)
#define PAR (am*(1-0.205f*0.205f))
#define VP 60
#define C 2990000

imm track[ll];
uint32_t start_index = 0;
float V[WW][HH];
imm position = imm(256, 256 - 150);
imm velosity = imm(VP, 0);
float dt = 0;

void rend(shwin* wnd) {
	hd.clear(0xFF00000F);

    //for(uint32_t x(WW);x--;)
    //    for(uint32_t y(HH);y--;)
    //        hd.draw(x, y, 0xFF000000 | (0x00FFFFFF&(uint32_t)(V[x][y])));

    imm acceleration = -imm(
        V[((uint32_t)position.x+1) % WW][((uint32_t)position.y  ) % HH] - V[((uint32_t)position.x) % WW][((uint32_t)position.y) % HH], 
        V[((uint32_t)position.x  ) % WW][((uint32_t)position.y+1) % HH] - V[((uint32_t)position.x) % WW][((uint32_t)position.y) % HH]);

    position = position + velosity*dt + acceleration*0.5f*dt*dt;
    if (position.x >= WW)
        position.x -= WW;
    if (position.x < 0)
        position.x += WW;
    if (position.y >= HH)
        position.y -= HH;
    if (position.y < 0)
        position.y += HH;
    velosity = velosity + acceleration*dt;
    if ((position - track[start_index]).length() > gg) {
        start_index = (start_index+1)%ll;
        track[start_index] = position;
    }

    hd.ring(position.x, position.y, 5, 0xFFFF0010);
    hd.ring(hd.width>>1, hd.height>>1, 10, 0xFFFFFF00);
	for (uint32_t u(start_index+1); u < start_index + ll; u++) {
        hd.line2p(track[u%ll].x, track[u%ll].y, track[(u+1)%ll].x, track[(u+1)%ll].y, 0xFF0000FF);

		//hd.circ(rand() % hd.width, rand() % hd.height, 64 + rand() % 64, 0xFF000000 + 0x00FFFF00 & (rand()));
		//hd.rect(rand() % hd.width, rand() % hd.height, 64 + rand() % 64, 64 + rand() % 64, 0xFF000000 + 0x00FFFF00 & (rand()));
		//uint32_t x = rand() % hd.width;
		//uint32_t y = rand() % hd.height;
		//hd.rect2p(x, y, x + rand() % 128, y + rand() % 128, 0xFF000000 + 0x00FFFF00 & (rand()));
		//hd.line(hd.width >> 1, hd.height >> 1, (rand() % hd.width) - (hd.width >> 1), (rand() % hd.height) - (hd.height >> 1), 0xFF000000 + 0x00FFFF00 & rand());
		//hd.line2p(hd.width >> 1, hd.height >> 1, (rand() % hd.width), (rand() % hd.height), 0xFF000000 + 0x00FFFF00 & rand());
	}
	StretchDIBits(wnd->hdc, 0, 0, wnd->width, wnd->height, 0, 0, hd.width, hd.height, hd.handle, &wnd->buf_info, DIB_RGB_COLORS, SRCCOPY);

	dt = ((float)(comp_mspf())) / 1000.0f;
	std::wstring s = L" s to frame: " + std::to_wstring(dt) + L" spf, speed: " + std::to_wstring(1.0f/dt) + L" fps ";
	TextOutW(wnd->hdc, 0, 0, s.c_str(), s.length());
}

int WinMain(HINSTANCE hinst, HINSTANCE prev_hinst, LPSTR cmd_line, int show_cmd) {

    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin),  _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    std::wcout << L"Введите параметр ae, для Меркурия примерно 14:\n";
    std::wcin >> ae;

    for(uint32_t x(WW);x--;)
        for(uint32_t y(HH);y--;) {
            V[x][y] = GSM * (-1/(imm(x + 0.5f - (WW>>1), y + 0.5f - (HH>>1)).length())
             + (PAR / (imm(x + 0.5f - (WW>>1), y + 0.5f - (HH>>1)).length2())) * (
                0.5f - GSM / ((imm(x + 0.5f - (WW>>1), y + 0.5f - (HH>>1)).length())*C)));
        }

	hd.init(WW, HH);
	wnd.init(hinst, L"Graphics test\0", rend, WW, HH);
	wnd.init_draw_area(hd.width, hd.height);
	wnd.show_window(show_cmd);

	MSG msg;
	do {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow(wnd.win, NULL, NULL, RDW_INTERNALPAINT);
	} while (msg.message != WM_QUIT);

	return msg.wParam;
}