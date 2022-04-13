#include <iostream>
#include <GL/freeglut.h>
#include <thread>
#include <time.h>
#include <vector>
#include <mutex>

using namespace std;

mutex datalocker;

struct clr { float r, g, b; };

bool needrefresh = false;
bool exitcall = false;
bool canwork = false;
bool cleanupcall = false;

int bsr = 0;

float ballsides = 8;

struct world
{
	float xmin, ymin, xmax, ymax;
	double dt;
} cwrld;

clock_t time_s = clock();

class bubble
{
private:
	float bx, by, kx, ky;
	float br;
	float spd;
	clr bclr;
public:
	void setbx(float val) { bx = val; }
	void setby(float val) { by = val; }
	void setkx(float val) { kx = val; }
	void setky(float val) { ky = val; }
	void setbr(float val) { br = val; }
	void setspd(float val) { spd = val; }
	void setbclr(clr val) { bclr = val; }
	float getbx() { return bx; }
	float getby() { return by; }
	float getkx() { return kx; }
	float getky() { return ky; }
	float getbr() { return br; }
	float getspd() { return spd; }
	clr getbclr() { return bclr; }

	void move()
	{
		bx += kx * spd*cwrld.dt;
		by += ky * spd*cwrld.dt;

		if ((bx - br) < cwrld.xmin)
		{
			setbx(cwrld.xmin + br);
			setkx(kx*(-1));
		}
		if ((bx + br) > cwrld.xmax)
		{
			setbx(cwrld.xmax - br);
			setkx(kx*(-1));
		}
		if ((by - br) < cwrld.ymin)
		{
			setby(cwrld.ymin + br);
			setky(ky*(-1));
		}
		if ((by + br) > cwrld.ymax)
		{
			setby(cwrld.ymax - br);
			setky(ky*(-1));
		}
	}

	bubble(float x, float y, float r, clr cclr, float bspd)
	{
		setbx(x); setby(y); setbr(r); setbclr(cclr); setspd(bspd);
		kx = rand() % 2; if (kx == 0) kx = -1;
		ky = rand() % 2; if (ky == 0) ky = -1;
	}
};

void cb_display();

vector <bubble> bubbles = {};

double getdt()
{
	clock_t ct = clock();
	double cdt = double(ct - time_s) / CLOCKS_PER_SEC;
	if (cdt!=0) 
		time_s = ct;
	return cdt;
}

void setupworld(float ww, float wh)
{
	cwrld.xmin = -1 * ww / 2;
	cwrld.ymin = -1 * wh / 2;
	cwrld.xmax = ww / 2;
	cwrld.ymax = wh / 2;
	//printf("World set: X: %f - %f; Y: %f - %f\n", cwrld.xmin, cwrld.xmax, cwrld.ymin, cwrld.ymax);
}

void drawcircle(float x, float y, float r, clr colour)
{
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < ballsides+1; i++)
	{
		/* x = radius * cos(angle)  
           y = radius * sin(angle)*/
		glColor3f(colour.r, colour.g, colour.b);
		float cx = r * cos( ((360 / ballsides)*(float)i) * 3.14 / 180) + x;
		float cy = r * sin( ((360 / ballsides)*(float)i) * 3.14 / 180) + y;
		if (i == 0)
		{
			glVertex2f(x, y);
		}
		if (i < ballsides)
		{
			glVertex2f(cx, cy);
		}
		if (i == ballsides)
		{
			cx = r + x;
			cy = y;
			glVertex2f(cx, cy);
		}

	}
	glEnd();	
}

void cb_reshape(int nw, int nh)
{
	setupworld(nw,nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, nw, nh);
	glOrtho((double)cwrld.xmin, (double)cwrld.xmax, (double)cwrld.ymin, (double)cwrld.ymax,-1,1);
	needrefresh = true;
}

void cb_idle()
{
	if (needrefresh)
	{
		needrefresh = false;
		cb_display();
	}

	if (exitcall)
	{
		exitcall = false;
		glutLeaveMainLoop();
	}
}


void cb_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (canwork)
	{
		glLineWidth(4);

		glColor3f(1, 0, 0);

		glBegin(GL_LINE_LOOP);
		glVertex2f(cwrld.xmin,cwrld.ymin);
		glVertex2f(cwrld.xmin, cwrld.ymax);
		glVertex2f(cwrld.xmax, cwrld.ymax);
		glVertex2f(cwrld.xmax, cwrld.ymin);
		glEnd();

		glLineWidth(4);

		glColor3f(0, 1, 0);

		glBegin(GL_LINE_LOOP);
		glVertex2f(cwrld.xmin/2, cwrld.ymin/2);
		glVertex2f(cwrld.xmin/2, cwrld.ymax/2);
		glVertex2f(cwrld.xmax/2, cwrld.ymax/2);
		glVertex2f(cwrld.xmax/2, cwrld.ymin/2);
		glEnd();

		glColor3f(0, 0, 1);

		glBegin(GL_LINES);
		glVertex2f(0, cwrld.ymin);
		glVertex2f(0, cwrld.ymax);
		glVertex2f(cwrld.xmax, 0);
		glVertex2f(cwrld.xmin, 0);
		glEnd();

		bsr = bubbles.size();
		if (bubbles.size() > 0)
		{
			datalocker.lock();
			for (int i = 0; i < bubbles.size(); i++)
			{
				if (bsr == bubbles.size())
				{
					drawcircle(bubbles[i].getbx(), bubbles[i].getby(), bubbles[i].getbr(), bubbles[i].getbclr());
				}
				else break;
			}
			datalocker.unlock();
		}
	}

	glutSwapBuffers();

}

void freeglutthread(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Flying Bubbles");
	glClearColor(0, 0, 0, 1);

	glutIdleFunc(cb_idle);
	glutReshapeFunc(cb_reshape);
	glutDisplayFunc(cb_display);
	glutMainLoop();
}

void logicthread()
{
	while (true)
	{
		if (exitcall) break;

		cwrld.dt = getdt();
		if (cwrld.dt>0)
		if (canwork)
		{
			if (bubbles.size() > 0)
			{
				datalocker.lock();
				for (int i = 0; i < bubbles.size(); i++)
				{
					bubbles[i].move();
				}
				datalocker.unlock();
			}
			needrefresh = true;
		}

		if (cleanupcall)
		{
			cleanupcall = false;
			canwork = false;
			datalocker.lock();
			for (int i = 0; i < bubbles.size(); i++)
			{
				bubbles[i].~bubble();
			}
			bubbles.clear();
			datalocker.unlock();
			canwork = true;
		}
	}
}

void spawnbubble()
{
	float r = 10 + rand() % 10;
	float wwid = cwrld.xmax - cwrld.xmin;
	float whei = cwrld.ymax - cwrld.ymin;
	float bpx = rand() % ((int)trunc(wwid - 2 * r)) + r - wwid/2;
	float bpy = rand() % ((int)trunc(whei - 2 * r)) + r - whei/2;
	float bbspd = 50 + rand() % 300;
	clr col = { float(rand() % 256) / 256,
				float(rand() % 256) / 256,
				float(rand() % 256) / 256 };
	datalocker.lock();
	bubbles.push_back(bubble(bpx, bpy, r, col, bbspd));
	datalocker.unlock();
}

int main(int argc, char **argv)
{
	srand(time(NULL));

	thread fgt(freeglutthread, argc, argv);
	thread lgc(logicthread);
	
	canwork = true;

	while (true)
	{
		printf("Input command:\n1 - Spawn entities.\n2 - Clear field.\n3 - Ball sides.\n4 - Exit.\n");
		int c = 0;
		cin >> c;
		if (c > 4) c = 4; if (c < 1) c = 1;
		if (c == 1)
		{
			canwork = false;
			printf("Input amount:\n");
			int n = 1;
			cin >> n;
			if (n < 1) n = 1;
			for (int i = 0; i < n; i++) spawnbubble();
			canwork = true;
		}

		if (c == 2)
		{
			cleanupcall = true;
		}

		if (c == 4)
		{
			canwork = false;
			exitcall = true;
			break;
		}
		if (c == 3)
		{
			canwork = false;
			printf("Input amount:\n");
			int n = 1;
			cin >> n;
			if (n < 3) n = 3;
			ballsides = n;
			canwork = true;
		}
	}
	fgt.join();
	lgc.join();
}