//Made by Paras Chopra
//See the particle systems tutorial at http://www.paraschopra.com/tutorials/particle-systems/
//Email: paars1987@gmail.com
//Email: paraschopra@paraschopra.com

#include<iostream.h>
#include<conio.h>
#include<stdlib.h>
#include<graphics.h>
#include<dos.h>

enum ParticleType { Create,Update };

struct Vector2d{
	double x;
	double y;
};

struct Particle {
	Vector2d Pos; //Position of the particle
	Vector2d Vel; //Velocity of the particle
	int age; //Current age of the particle
	int LifeSpan; //Age after which the particle dies
	int color;
	int size;
};



class ParticleSystem{

	protected:
		Particle * Particles;
		long ParticleNum;
		Vector2d Vel;
		Vector2d Pos1;
		Vector2d Pos2;
		//Vector2d * Forces;
		//long ForcesNum;
		Vector2d TotForce;
		int LifeSpan;
		int Color;
		int Size;
		ParticleType Particle_Type;

	void InitParticles(){
			for(int i=0;i<ParticleNum;i++)
				InitParticle(i);
		}

	void InitParticle(int i){
		Particles[i].Pos.x = random(Pos2.x-Pos1.x+1)+Pos1.x; //Random X position inside the rectangle
		Particles[i].Pos.y = random(Pos2.y-Pos1.y+1)+Pos1.y; //Random Y position inside the rectangle
		Particles[i].Vel.x = (random(100+1)/100.0) * Vel.x - Vel.x/2; //Random velocity
		Particles[i].Vel.y = (random(100+1)/100.0) * Vel.y - Vel.y/2; //Random velocity
		Particles[i].color=Color;
		Particles[i].size=Size;
		Particles[i].age=0; //New born baby
		Particles[i].LifeSpan=random(LifeSpan)+1; //Random lifespan
	}


	public:
		//Call this function before doing anything
		void InitGraphics(){

			int gdriver = DETECT, gmode, errorcode;
			/* initialize graphics and local variables */
			initgraph(&gdriver, &gmode, "");
			/* read result of initialization */
			errorcode = graphresult();
			if (errorcode != grOk)  /* an error occurred */
			{
				cout<<"Graphics error: "<<grapherrormsg(errorcode)<<"\n";
				cout<<"Press any key to halt:";
				getch();
				exit(1); /* terminate with an error code */
			}
				}


		
		//Pos1 and Pos2 indicate the position of the two opposite corners of the rectangle between which particles are generated
		//This rectangle basically restricts the initial position of a newly generated particle within it
		void Init(long num_part,long num_forces, Vector2d Forces[], ParticleType part_type,Vector2d vel, Vector2d pos1,Vector2d pos2, int lifespan, int color, int size){
			Particles = new Particle[num_part]; //num_part is number of particles
			ParticleNum=num_part;
			for(int i=0;i<num_forces;i++){ //num_forces is number of forces
				TotForce.x+=Forces[i].x;
				TotForce.y+=Forces[i].y;
			}
			Particle_Type=part_type;
			Vel=vel;
			Pos1=pos1;
			Pos2=pos2;
			LifeSpan=lifespan;
			Color=color;
			Size=size;
			randomize();
			InitParticles();
		}

		void ModifyForce(long num_forces, Vector2d Forces[]){
		     for(int i=0;i<num_forces;i++){
				TotForce.x+=Forces[i].x;
				TotForce.y+=Forces[i].y;
			}
		}


	void Run(){
		while(!kbhit()){
			delay(10);
			cleardevice();
			//delay(10);
			for(int i=0;i<ParticleNum;i++){
				if(Particles[i].age >= 0){
				setfillstyle(1,Particles[i].color);
				setcolor(Particles[i].color);
				//circle((int)Particles[i].Pos.x,(int)Particles[i].Pos.y,Particles[i].size);
				fillellipse((int)Particles[i].Pos.x,(int)Particles[i].Pos.y,Particles[i].size,Particles[i].size);
				Particles[i].Vel.x+=TotForce.x;
				Particles[i].Vel.y+=TotForce.y;
				Particles[i].Pos.x+=Particles[i].Vel.x;
				Particles[i].Pos.y+=Particles[i].Vel.y;
				Particles[i].age++;
				if(Particles[i].age > Particles[i].LifeSpan){
				if(Particle_Type == Create) {
				InitParticle(i);
				 } else {
				 Particles[i].age = -1;
				 }   }

				}
			}
		}   }
	~ParticleSystem() { delete Particles; }
};

class Bouncy : public ParticleSystem{
	 public:
	 void Run(){
		while(!kbhit()){
			delay(50);
			cleardevice();
			//delay(50);
			for(int i=0;i<ParticleNum;i++){
				if(Particles[i].age >= 0){
				setfillstyle(1,Particles[i].color);
				setcolor(Particles[i].color);
				//circle((int)Particles[i].Pos.x,(int)Particles[i].Pos.y,Particles[i].size);
				fillellipse((int)Particles[i].Pos.x,(int)Particles[i].Pos.y,Particles[i].size,Particles[i].size);
				Particles[i].Vel.x+=TotForce.x;
				Particles[i].Vel.y+=TotForce.y;
				Particles[i].Pos.x+=Particles[i].Vel.x;
				Particles[i].Pos.y+=Particles[i].Vel.y;
				Particles[i].age++;
				if(Particles[i].Pos.y>getmaxy() || Particles[i].Pos.y<0) Particles[i].Vel.y*=-0.9;
				if(Particles[i].Pos.x>getmaxx() || Particles[i].Pos.x<0) Particles[i].Vel.x*=-0.9;
				if(Particles[i].age > Particles[i].LifeSpan) {
				if(Particle_Type == Create) {
				InitParticle(i);
				 } else {
				 Particles[i].age = -1;
				 }
				}

				}
			}
		}   }
	 };


void main(){
	Bouncy n;
	//ParticleSystem n;
	int n_forces=1;
	Vector2d f[1];
	f[0].x=0.0;
	f[0].y=0.5;
	Vector2d vel,pos1,pos2;
	vel.x=5;
	vel.y=5;

	n.InitGraphics(); //Most important thing to do
	setbkcolor(BLACK);
	pos1.x=getmaxx()/2;
	pos1.y=getmaxy()/2;
	//pos2.x=getmaxx()/2;
	pos2.x=getmaxx()/2;
	//pos2.y=getmaxy()/2;
	pos2.y=getmaxy()/2;
	n.Init(1000,n_forces,f,Create,vel,pos1,pos2,100,EGA_GREEN+EGA_RED,1);
	n.Run();

	getch();
}




