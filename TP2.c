#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h> 

#define PI 3.14159265358979323846

typedef struct Surface
{
    int width;  ///largeur
    int height; ///hauteur
    int depth; /// bps
    double *data; ///pixels
}SURFACE;

int iclamp ( int x,int min,int max){
    if (x<min){
        return min;
    }
    if(x>max){
        return max;
    }
    return x;
}


void surface(SURFACE *s,int width,int height){
    s->data = (double *)malloc(3* width * height * sizeof(double));
    if(s->data !=NULL){
        s->width = width;
        s->height = height;
        s->depth = 8;
    }
}

void _surface(SURFACE *s){
    free(s->data);
}

double *at(SURFACE *s,int x,int y){
    if(x < 0 || x >= s->width) return NULL;
    if(y < 0 || y >= s->height) return NULL;
    return s->data + y * s -> width*3 +(x*3);
}
// Retourne l'adresse du pixel en (x,y) ou NULL si le pixel n'existe pas

void point(SURFACE *s, int x,int y,double r,double g,double b){
    double *pos = at(s,x,s->height-1-y);
    if (pos != NULL){
        pos[0] = r;
        pos[1] = g;
        pos[2] = b;
    }
}
// modifier un pixel

 /*void fill(SURFACE *s,double r,double g,double b){


    for (double *i = s-> data, *e=s-> data + (s->width * s->height*3)/2 ;i!=e;i+=3){
        *i=r;
        *(i+1)=g;
        *(i+2)=b;
        printf("%d\n",*i);
    }
 }*/

 void fill(SURFACE *s){

    for (double *i = s->data, *e=s->data + (3*(s->width * s->height))/2; i!=e;i+=3){
        *i=0.;
        *(i+1)=0.71;
        *(i+2)=1.;
    }
    for (double *i = s->data +(3*(s->width * s->height))/2, *e=s->data + (3*(s->width * s->height)); i!=e;i+=3){
    *i=0.;
    *(i+1)=0.9;
    *(i+2)=0.2;
    }        
 }
// remplit le fond pixel par pixel

 int pgm_write(SURFACE *s, FILE *f){
    int max =1 << s->depth;

    int count = fprintf(f,"P3\n# pgm_write\n%d %d\n%d\n",s->width,s->height,max-1);

    int cr = s->width;
    for (double *i = s-> data, *e = s->data + (3*(s->width * s->height)); i!=e;i++){
        count +=fprintf(f,"%d",iclamp(*i *max,0,max-1));

        if(--cr){
            count+=fprintf(f," ");
        }
        else{
            cr=s->width;
            count+=fprintf(f,"\n");
        }
    }

    return count;
}
// retourne le nombre d'octets écrits en format pgm

int pgm_read(SURFACE *s,FILE *f){
    if (fgetc(f) != 'P')return 0;
    if (fgetc(f) != '2')return 0;
    if (fgetc(f) != '\n')return 0;
    
    char c;
    while((c=fgetc(f))=='#') 
        while (fgetc(f)!='\n');
    ungetc(c,f);

    int width,height,max;
    if(fscanf(f,"%d%d%d",&width,&height,&max)!=3)
    return 0;

    SURFACE surf;
    surface(&surf,width,height);
    if(surf.data == NULL) return 0;

    for (double *i = surf.data, *e =surf.data +surf.width *surf.height; i !=e;++i){

        int value;
        
        if (fscanf(f,"%d",&value)!=1){

            _surface(&surf);
            return 0;
        }

        *i = (double)value /max;
    }

    surf.depth=8;

    _surface(s);

    *s = surf;

    return 1; 

}
// lecture d'un fichier formaté en format pgm


void segment(SURFACE *s,int x1,int y1,int x2,int y2, double r,double g,double b){

    int A=(y2-y1)/(x2-x1);
    int B= y1-A*x1;
    for (int x=x1;x<x2;x++){
        int y=floor(A*x+B);
        //printf("%d\n",y);
        point(s,x,y,r,g,b);
    }
}

void segment_sauf_1er_point(SURFACE *s,double x1,double y1,double x2,double y2,double R,double G,double B){
    int dx,dy;
    dx=x2>x1? x2-x1 : x1-x2;
    dy=y2>y1? y2-y1 : y1-y2;
    if (dx>dy){
        double a=(y2-y1)/(x2-x1);
        double b= y1-a*x1;
        for (double x=x1;x<x2;x++){
            double y=floor(a*x+b);
            //printf("erreur boucle 1\n");
            //printf("RGB : %lf %lf %lf\n",R,G,B);
            point(s,x,y,R,G,B);
    }
    }
    else{
        double a=(y2-y1)/(x2-x1);
        double b=y1-a*x1;
        if(y1<y2){
        for (double y=y1;y<y2;y++){
            double x=floor((y-b)/a);
            //printf("%d\n",(y-b)/a);
            //printf("erreur boucle 2\n");
            point(s,x,y,R,G,B);
            }
        }
        else{
            for (double y=y1;y>y2;y--){
            double x=floor((y-b)/a);
            //printf("%d\n",(y-b)/a);
            //printf("erreur boucle 3\n");
            point(s,x,y,R,G,B);
            }
        }
    } 
}

void epicycloide(SURFACE *s,int x,int y,int RAY, int ray,double r,double g,double b){
    //x et y coordonnés de départ
    //RAYON rayon extérieur
    //rayon rayon intérieur
    double x0, y0, x1, y1;
    x0 = (RAY+ray)*cos(0)+ray*cos(0);
    y0 = (RAY+ray)*sin(0)+ray*sin(0);
    
    for (double i=0.001;i<PI;i+=0.001){
        x1 = (RAY+ray)*cos(i)+(ray*cos(i));
        y1 = (RAY+ray)*sin(i)+(ray*sin(i));
        segment_sauf_1er_point(s,x0,y0,x1,y1,r,g,b);
        
        //printf("%lf \n",i);
        x0=x1;
        y0=y1;
    }
    printf("%lf %lf\n", x1,y1);
    // x = (R+r)Cos(t) + rCos(kt) 
    // y = (R+r)Sin(t) + rSin(kt)
    // R etant le rayon du grand cercle et r le rayon des petales
}


void epicycloide2(SURFACE *s,int x,int y,int RAY, int ray,double r,double g,double b){
    //x et y coordonnés de départ
    //RAYON rayon extérieur
    //rayon rayon intérieur
    double x0, y0, x1, y1;
    x0 = 300+(50+40)*cos(0)+40*cos(5*(90/40)*0);
    y0 = 300+(50+40)*sin(0)+40*sin(5*(90/40)*0);
    
    for (double i=0.001;i<=2*PI;i+=0.001){
        x1 = 300+(50+40)*cos(i)+40*cos(5*(90/40)*i);
        y1 = 300+(50+40)*sin(i)+40*sin(5*(90/40)*i);
        //segment(s,x0,y0,x1,y1,r,g,b);
        //printf("%lf %lf\n", x1-x0,y1-y0);
        segment_sauf_1er_point(s,x0,y0,x1,y1,r,g,b);

        //printf("%lf \",i);

        x0=x1;
        y0=y1;
    }
    //printf("%lf %lf\n", x1,y1);
    // x = (R+r)Cos(t) + rCos(kt) 
    // y = (R+r)Sin(t) + rSin(kt)
    // R etant le rayon du grand cercle et r le rayon des petales
}


/*
void triangle(SURFACE *s,int x1,int y1,int x2,int y2,int x3,int y3){
    segment_sauf_1er_point(s,x1,y1,x2,y2);
    segment_sauf_1er_point(s,x2,y2,x3,y3);
    segment_sauf_1er_point(s,x3,y3,x1,y1);
}

void segement_gris(SURFACE *s,int x1,int y1,double g1 ,int x2,int y2,double g2){
 int dx,dy;
 double g=g1;
    dx=x2>x1?x2-x1:x1-x2;
    dy=y2>y1?y2-y1:y1-y2;
    if (dx>dy){
        int a=(y2-y1)/(x2-x1);
        int b= y1-a*x1;
        for (int x=x1+1;x<x2;x++){
            int y=floor(a*x+b);
            g+=1/(x2-x1);
            point(s,x,y,g);
    }
    }
    else{
        int a=(y2-y1)/(x2-x1);
        int b=y1-a*x1;
        if(y1<y2){
        for (int y=y1+1;y<=y2;y++){
            int x=floor((y-b)/a);
            g+=(double)1/(y2-y1);
            printf("%f\n",g);
            point(s,x,y,g);
        }
        }
        else{
            for (int y=y1+1,g=g1;y>=y2;y--,g+=0.1){
            int x=floor((y-b)/a);
            g+=(double)1/(y2-y1);
            point(s,x,y,g);
            }
        }
    } 
}

void tracertriangle(SURFACE *s,int x1,int y1,int x2,int y2,int x3,int y3){
    int xi;
    double a=(x1-x3)/(y1-y3);
    int b=y3-a*x3;
    xi=(y3-b)/a;
    printf("%d",xi);
    //triangle(s,x2,y2,xi,y2,x3,y3);

}
*/
int main(){

    SURFACE surf={0};
    
    surface(&surf,1000,1000);
    
    assert(surf.data!=NULL);
    
    fill(&surf);
    printf("ok\n");
    //epicycloide(&surf,500,500,1000,500,1.0,0.0,0.0);
    epicycloide2(&surf,300,500,1000,500,1.0,0.0,0.0);
    printf("ok\n");
    FILE *output = fopen("fleurs.ppm","w");
    
    assert(output != NULL);
    
    pgm_write(&surf,output);
    
    fclose(output);

    _surface(&surf);

    return 0;

}