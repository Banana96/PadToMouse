#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Xinput.h>
#include <iostream>

#pragma comment(lib, "XInput.lib")

#define B_DUP XINPUT_GAMEPAD_DPAD_UP
#define B_DDOWN XINPUT_GAMEPAD_DPAD_DOWN
#define B_DLEFT XINPUT_GAMEPAD_DPAD_LEFT
#define B_DRIGHT XINPUT_GAMEPAD_DPAD_RIGHT
#define B_START XINPUT_GAMEPAD_START
#define B_BACK XINPUT_GAMEPAD_BACK
#define B_LS XINPUT_GAMEPAD_LEFT_THUMB
#define B_RS XINPUT_GAMEPAD_RIGHT_THUMB
#define B_LB XINPUT_GAMEPAD_LEFT_SHOULDER
#define B_RB XINPUT_GAMEPAD_RIGHT_SHOULDER
#define B_A XINPUT_GAMEPAD_A
#define B_B XINPUT_GAMEPAD_B
#define B_X XINPUT_GAMEPAD_X
#define B_Y XINPUT_GAMEPAD_Y

using namespace std;

struct Point {
	float x, y;

	Point(float x, float y) {
		this->x = x;
		this->y = y;
	}
};

class Controller {
private:
	XINPUT_STATE state;
	float deadzone;

public:
	Controller(float dz = 0.3f) {
		deadzone = dz;
	}

	void setDeadzone(float dz) {
		deadzone = dz;
	}

	void updateState() {
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(0, &state);
	}

	XINPUT_STATE getState() {
		return state;
	}

	Point getLSPos() {
		Point pt(
			(float) getState().Gamepad.sThumbLX / 32768,
			(float) getState().Gamepad.sThumbLY / -32768
		);

		if(abs(pt.x) > deadzone | abs(pt.y) > deadzone) {
			return pt;
		} else {
			return Point(0, 0);
		}
	}

	Point getRSPos() {
		Point pt(
			(float) getState().Gamepad.sThumbRX / 32768,
			(float) getState().Gamepad.sThumbRY / -32768
		);

		if(abs(pt.x) > deadzone | abs(pt.y) > deadzone) {
			return pt;
		}
		else {
			return Point(0, 0);
		}
	}

	float getLT() {
		return (int) getState().Gamepad.bLeftTrigger / 255;
	}

	float getRT() {
		return (int) getState().Gamepad.bRightTrigger / 255;
	}

	bool isPressed(int button) {
		return getState().Gamepad.wButtons & button;
	}

	bool isConnected() {
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		return XInputGetState(0, &state) == ERROR_SUCCESS;
	}

	void vibrate(int left = 0, int right = 0) {
		XINPUT_VIBRATION vib;
		ZeroMemory(&vib, sizeof(XINPUT_VIBRATION));
		vib.wLeftMotorSpeed = left;
		vib.wRightMotorSpeed = right;
		XInputSetState(0, &vib);
	}
};

class Mouse {
private:
	HCURSOR handle;
	POINT p;
	
public:
	Mouse() {
		SetCapture(NULL);
		updatePos();
	}

	~Mouse() {
		ReleaseCapture();
	}

	void updatePos() {
		GetCursorPos(&p);
	}

	int getX() {
		return p.x;
	}

	int getY() {
		return p.y;
	}

	void moveTo(float x, float y) {
		p.x = (int) x;
		p.y = (int) y;
		SetCursorPos(this->p.x, this->p.y);
	}

	void moveBy(float dx = 0.f, float dy = 0.f) {
		p.x += (int) dx;
		p.y += (int) dy;
		SetCursorPos(p.x, p.y);
	}

	void setLeft(bool clicked) {
		INPUT input;

		ZeroMemory(&input, sizeof(INPUT));
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = clicked ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(INPUT));
	}

	void setRight(bool clicked) {
		INPUT input;

		ZeroMemory(&input, sizeof(INPUT));
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = clicked ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
		SendInput(1, &input, sizeof(INPUT));
	}
};

int main() {
	Controller *ctrl = new Controller();
	Mouse *mouse = new Mouse();

	float speed = 8.f;

	bool finish = false;
	bool lastA = false;
	bool lastB = false;
	bool lastY = false;

	while(!finish) {
		if(ctrl->isConnected()) {
			ctrl->updateState();
			mouse->updatePos();

			Point lsp = ctrl->getLSPos();
			float boost = ctrl->getRT() > 0.1f ? 3.5f * ctrl->getRT() : 1.f;
			float precise = boost == 1.f ? (ctrl->getLT() > 0.1f ? 0.4f * ctrl->getLT() : 1.f) : 1.f;

			mouse->moveBy(lsp.x * speed * boost * precise, lsp.y * speed * boost * precise);

			if(lastA != ctrl->isPressed(B_A)) {
				lastA = ctrl->isPressed(B_A);
				mouse->setLeft(ctrl->isPressed(B_A));
			}

			if(lastB != ctrl->isPressed(B_B)) {
				lastB = ctrl->isPressed(B_B);
				mouse->setRight(ctrl->isPressed(B_B));
			}

			if(lastY != ctrl->isPressed(B_Y)) {
				lastY = ctrl->isPressed(B_Y);

			}

			if(ctrl->isPressed(B_BACK) & ctrl->isPressed(B_START)) {
				finish = true;
			}

			Sleep(16);
		} else {
			Sleep(3000);
		}
	}

	delete ctrl;
	delete mouse;

	return 0;
}