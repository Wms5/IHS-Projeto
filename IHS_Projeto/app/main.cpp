//COMPILAR: gcc -o main main.cpp -lallegro -lallegro_ttf -lallegro_font -lallegro_image
//RODAR: ./main

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
//#include <math.h>
#include <time.h>

#define max_tiros 3
#define max_inimigos 20
#define max_blocos 20

int nivel = 2;

unsigned char hexdigit[] = {0x3F, 0x06, 0x5B, 0x4F,
                            0x66, 0x6D, 0x7D, 0x07,
                            0x7F, 0x6F, 0x77, 0x7C,
			                0x39, 0x5E, 0x79, 0x71};

typedef struct VECTOR
{
 	int option; 	//1: 7seg, 2: ledG, 3:ledR
 	unsigned data;  //dados
}Vect;

void setPontos(int p, int dev)
{
  	int i1, i2, i3, i4, k;
  	Vect v;



  	v.option = 1;
  	i1 = p/1000;
  	p = p%1000;
  	i2 = p/100;
  	p = p%100;
  	i3 = p/10;
  	p = p%10;
  	i4 = p;
  	k =  hexdigit[i4 & 0xF] | (hexdigit[i3 & 0xF] << 8)| (hexdigit[i2 & 0xF] << 16)| (hexdigit[ i1 & 0xF] << 24);
  	k = ~k;
	v.data = k;
  	write(dev, (const char*) &v, 4);
}

void level(int dev)
{
  	Vect v;
  	v.option = 2;
  	v.data = 0;

  	for(int i = 0; i < nivel; i++)
  	{
    	v.data <<= 2;
    	v.data |= 3;
  	}
  	write(dev, (const char*) &v, 4);
}

void piscaGameOver(int dev)
{
    int k = 10;
    Vect v;
    v.option = 3;

    while(k--)
    {
    	v.data = (k & 1 ? 0xFFFFFFFF : 0x00000000);
      	write(dev, (const char*) &v, 4);
      	usleep(150000); // um milisegundo
    }
}

void setCombo(int p, int dev)
{
  	int i1, i2, i3, i4, k;
  	Vect v;
  	v.option = 4;
  	i1 = p/1000;
  	p = p%1000;
  	i2 = p/100;
  	p = p%100;
  	i3 = p/10;
  	p = p%10;
  	i4 = p;
  	k =  hexdigit[i4 & 0xF] | (hexdigit[i3 & 0xF] << 8)| (hexdigit[i2 & 0xF] << 16)| (hexdigit[ i1 & 0xF] << 24);
  	k = ~k;

  	v.data = k;
  	//write(dev, &k, 4);
  	write(dev, (const char*) &v, 4);
}

//---------------------------------------------------------começa aqui o jogo---------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------
const int LARGURA_TELA = 640;
const int ALTURA_TELA = 240;

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
ALLEGRO_FONT *fonte_over = NULL;
ALLEGRO_FONT *fonte_pontos = NULL;
clock_t init, final;

bool inicializar();

int min(int val1, int val2)
{
	if(val1 <= val2)
	{
		return val1;
	}
	return val2;
}

int max(int val1, int val2)
{
	if(val1 >= val2)
	{
		return val1;
	}
	return val2;
}

enum Estado{
    andando,
    pulando,
    dash
};

enum Tipo_inimigo{
    atirador
};

enum Tipo_blocos{
    a,
    b,
    d,
    f,
    h
};

struct Blocos{
    int pos_x;
    int frame_espinho;
    bool ponto;
    Tipo_blocos tipo;
};

Blocos lista_blocos[max_blocos];

struct Inimigos{
    int pos_x;
    int pos_y;
    int tam_x;
    int tam_y;
    int torre;
    int random;
    bool municao;
    bool passou;
    Tipo_inimigo tipo;
};

Inimigos lista_inimigos[max_inimigos];

struct Tiro{
    int pos_x;
    int pos_y;
    bool passou;
    bool megaman;
};

//bitmaps
ALLEGRO_BITMAP *megaman[11];
ALLEGRO_BITMAP *megaman_atirando[11];
ALLEGRO_BITMAP *mega_pulo[5];
ALLEGRO_BITMAP *mega_pulo_atirando[5];
ALLEGRO_BITMAP *tiro;
ALLEGRO_BITMAP *tiro_atirador;
ALLEGRO_BITMAP *inimigo_atirador[3];
ALLEGRO_BITMAP *chao[2];
ALLEGRO_BITMAP *fundo;
ALLEGRO_BITMAP *espinho;
ALLEGRO_BITMAP *espinho_menor;
ALLEGRO_BITMAP *painel;
ALLEGRO_BITMAP *topo;
ALLEGRO_BITMAP *the_end;

Estado estado = andando;
bool gameover = false;
//bool winterbells = false;
bool tiros = true;

float fps = 0.01;
int tempo = 0;
int velocidade_pulo = 3;
int velocidade_tiro = 5;
int velocidade_animacao = 7;
int velocidade_inimigos = 2;
int frequencia_obstaculos = 160; //40
int frequencia_monstros = 2;
int frequencia_tiros = 2;
int tempo_atirando = 15;
int tempo_dash = 75;
int tempo_no_ar = 35;
int tempo_espinho = 25;
int dificuldade;

bool atirando = false;
int atirando_tempo = 0;
int dash_tempo = 0;
Tiro atirados[10];

int frame_pulando = 0;
int frame_andando = 0;
int frame_dash = 0;
int altura_pulada = 0;
bool subindo = true;
bool segurando = false;
int altura_pulo = 125;
int qtd_tiros = 0;
int pontuacao = 0;
int melhor_pontuacao = 0;
int combo = 0;
int max_combo = 0;
int combo_das_balas = 0;

int altura_andando = 168;
int altura_pulando = 154;
int altura_dash = 174;
int frame_no_ar = 0;

void criar_tiro(int x, int y){
    bool criado = false;
    for (int i = 0; i < max_tiros && !criado; i++){
        if (atirados[i].pos_x == 999){
            if (x == 0){
                atirados[i].pos_x = 108;
                if (estado == andando)
                    atirados[i].pos_y = altura_andando+15;
                else
                    atirados[i].pos_y = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+12;
                atirados[i].megaman = true;
            }
            else{
                atirados[i].pos_x = x;
                atirados[i].passou = false;
                atirados[i].pos_y = y+13;
                atirados[i].megaman = false;
            }
            criado = true;
        }
    }
}

void criar_inimigo(int altura, int torre){
    bool criado = false;
    for (int i = 0; i < max_inimigos && !criado; i++)
        if (lista_inimigos[i].pos_x == 999){
            lista_inimigos[i].pos_x = 650;
            lista_inimigos[i].pos_y = 165-45*altura;
            lista_inimigos[i].tam_x = 39;
            lista_inimigos[i].tam_y = 43;
            lista_inimigos[i].random = rand()%200;
            lista_inimigos[i].tipo = atirador;
            lista_inimigos[i].torre = torre;
            lista_inimigos[i].passou = false;
            if (!(rand()%frequencia_tiros) && tiros)
                lista_inimigos[i].municao = true;
            else
                lista_inimigos[i].municao = false;
            criado = true;
        }
}

int criar_blocos(int tipo){
    bool criado = false;
    int torre = -1;
    for (int i = 0; i < max_blocos && !criado; i++){
        if(lista_blocos[i].pos_x == 999){
            if (tipo == 5)
                lista_blocos[i].tipo = d;
            else if (tipo == 4){
                lista_blocos[i].pos_x = 650;
                lista_blocos[i].tipo = f;
                int k = criar_blocos(5);
                if (!(rand()%frequencia_monstros))
                    criar_inimigo(0, k);
                else
                    lista_blocos[k].frame_espinho = 0;
            }
            else if (tipo == 3)
                lista_blocos[i].tipo = h;
            else if (tipo == 2)
                lista_blocos[i].tipo = a;
            else if (tipo == 1)
                lista_blocos[i].tipo = b;

            lista_blocos[i].pos_x = 650;
            lista_blocos[i].frame_espinho = -1;
            lista_blocos[i].ponto = true;
            if (tipo == 4)
                lista_blocos[i].ponto = false;
            criado = true;
            torre = i;
        }
    }
    return torre;
}

void criar_obstaculos(){
    if (!(tempo%(frequencia_obstaculos*2/velocidade_inimigos))){
        int torre = -1;
        int altura = 0;
        int tipo = rand()%3;
        if (tipo < 5){
            switch (tipo){
            case 1:
                altura = 1;
                break;
            case 2:
                altura = 2;
                break;
            case 3:
                altura = 0;
                break;
            case 4:
                altura = 2;
                break;
            }
            if (tipo != 0)
                torre = criar_blocos(tipo);
            if (!(rand()%frequencia_monstros))
                criar_inimigo(altura, torre);
            else
                lista_blocos[torre].frame_espinho = 0;
        }
    }
}

void imprimir_tiro(){
    for (int i = 0; i < max_tiros; i++)
    if (atirados[i].pos_x < 999){
        if(atirados[i].megaman)
            al_draw_bitmap(tiro, atirados[i].pos_x, atirados[i].pos_y, 0);
        else
            al_draw_bitmap(tiro_atirador, atirados[i].pos_x, atirados[i].pos_y, 0);
    }
}

void imprimir_inimigo(){
    for (int i = 0; i < max_inimigos; i++){
        if (lista_inimigos[i].pos_x < 999){
            if(lista_inimigos[i].tipo == atirador){
                if(lista_inimigos[i].municao)
                    al_draw_bitmap(inimigo_atirador[0], lista_inimigos[i].pos_x, lista_inimigos[i].pos_y, 0);
                else
                    al_draw_bitmap(inimigo_atirador[1], lista_inimigos[i].pos_x, lista_inimigos[i].pos_y, 0);
            }
        }
    }
}

void imprimir_espinhos(int altura, int i){
    if (lista_blocos[i].frame_espinho == 0){
        al_draw_bitmap(topo, lista_blocos[i].pos_x, altura-30, 0);
        if (lista_blocos[i].frame_espinho < 0)
            lista_blocos[i].frame_espinho = 0;
    }
    else if (lista_blocos[i].frame_espinho > 0){
        float add = (1.0-((float)lista_blocos[i].frame_espinho/(float)tempo_espinho));
        al_draw_bitmap(topo, lista_blocos[i].pos_x, (int)((float)altura-15.0*add), 0);
        lista_blocos[i].frame_espinho-=velocidade_inimigos;
        if (lista_blocos[i].frame_espinho < 0)
            lista_blocos[i].frame_espinho = 0;
    }
}

void imprimir_espinhos_invertido(int i){
    if (lista_blocos[i].frame_espinho == 0){
        al_draw_bitmap(topo, lista_blocos[i].pos_x, 153+15, ALLEGRO_FLIP_VERTICAL);
        if (lista_blocos[i].frame_espinho < 0)
            lista_blocos[i].frame_espinho = 0;
    }
    else if (lista_blocos[i].frame_espinho > 0){
        float add = (1.0-((float)lista_blocos[i].frame_espinho/(float)tempo_espinho));
        al_draw_bitmap(topo, lista_blocos[i].pos_x, (int)((float)153+15.0*add), ALLEGRO_FLIP_VERTICAL);
        lista_blocos[i].frame_espinho-=velocidade_inimigos;
        if (lista_blocos[i].frame_espinho < 0)
            lista_blocos[i].frame_espinho = 0;
    }
}

void imprimir_correcao_chao(int i){
    al_draw_bitmap(chao[0], lista_blocos[i].pos_x, 208, 0);
    al_draw_bitmap(chao[0], lista_blocos[i].pos_x+16, 208, 0);
    al_draw_bitmap(chao[0], lista_blocos[i].pos_x+32, 208, 0);
    al_draw_bitmap(chao[1], lista_blocos[i].pos_x, 214, 0);
    al_draw_bitmap(chao[1], lista_blocos[i].pos_x+16, 214, 0);
    al_draw_bitmap(chao[1], lista_blocos[i].pos_x+32, 214, 0);
}

void imprimir_blocos(){
    for (int i = 0; i < max_blocos; i++){
        if (lista_blocos[i].pos_x < 999){
            if(lista_blocos[i].tipo == a){
                int altura = 118;//128
                imprimir_espinhos(altura+50, i);
                for (int j = 0; j < 5; j++)
                    al_draw_bitmap(espinho, lista_blocos[i].pos_x, altura+j*19, 0);
                imprimir_correcao_chao(i);
            }
            else if(lista_blocos[i].tipo == b){
                int altura = 162;//169
                imprimir_espinhos(altura+50, i);
                for (int j = 0; j < 3; j++)
                    al_draw_bitmap(espinho, lista_blocos[i].pos_x, altura+j*19, 0);
                imprimir_correcao_chao(i);
            }
            else if(lista_blocos[i].tipo == h){
                int altura = -21;//169
                imprimir_espinhos_invertido(i);
                for (int j = 0; j < 10; j++)
                    al_draw_bitmap(espinho, lista_blocos[i].pos_x, altura+j*19, 0);
            }
            else if(lista_blocos[i].tipo == f){
                int altura = 118;
                imprimir_espinhos(altura+50, i);
                al_draw_bitmap(espinho, lista_blocos[i].pos_x, altura+0*19, 0);
                al_draw_bitmap(espinho, lista_blocos[i].pos_x, altura+1*19, 0);
            }
            else if(lista_blocos[i].tipo == d){
                int altura = 118;
                imprimir_espinhos_invertido(i);
                al_draw_bitmap(espinho_menor, lista_blocos[i].pos_x, altura+2*19, 0);
            }
        }
    }
}

void atualizar_tiro(){
    for(int i = 0; i < max_tiros; i++)
    if (atirados[i].pos_x < 999){
        if (atirados[i].megaman){
            atirados[i].pos_x += velocidade_tiro;
            if (atirados[i].pos_x > LARGURA_TELA){
                atirados[i].pos_x = 999;
                qtd_tiros--;
            }
        }
        else{
            atirados[i].pos_x -= velocidade_tiro;
            if (atirados[i].pos_x < -20)
                atirados[i].pos_x = 999;
            else if (atirados[i].pos_x <= 75 && !atirados[i].passou){
                atirados[i].passou = true;
                combo_das_balas++;
            }
        }
    }
}

void atualizar_inimigos(){
    for (int i = 0; i < max_inimigos; i++){
        if (lista_inimigos[i].pos_x < 999){
            lista_inimigos[i].pos_x -= velocidade_inimigos;
            if (lista_inimigos[i].pos_x <= -50)
                lista_inimigos[i].pos_x = 999;
            else if (lista_inimigos[i].pos_x <= 75 && !lista_inimigos[i].passou){
                lista_inimigos[i].passou = true;
            }
            if (lista_inimigos[i].municao && lista_inimigos[i].pos_x < 350 + lista_inimigos[i].random){
                lista_inimigos[i].municao = false;
                criar_tiro(lista_inimigos[i].pos_x, lista_inimigos[i].pos_y);
            }
        }
    }
}

void atualizar_blocos(){
    for (int i = 0; i < max_blocos; i++){
        if (lista_blocos[i].pos_x < 999){
            lista_blocos[i].pos_x -= velocidade_inimigos;
            if (lista_blocos[i].pos_x <= -50)
                lista_blocos[i].pos_x = 999;
            if (lista_blocos[i].pos_x <=75 && lista_blocos[i].ponto){
                lista_blocos[i].ponto = false;
                pontuacao+=velocidade_inimigos;
            }
        }
    }
}

void colisao_tiro_no_inimigo(){
    for (int i = 0; i < max_tiros; i++){
        if (atirados[i].megaman && atirados[i].pos_x < 999){
            for (int j = 0; j < max_inimigos; j++){
                if (lista_inimigos[j].pos_x < 999){
                    int p1tx, p1ty, p4tx, p4ty;
                    int p1ex, p1ey, p4ex, p4ey;
                    p1tx = atirados[i].pos_x+1;
                    p1ty = atirados[i].pos_y+1;
                    p4tx = atirados[i].pos_x+10;
                    p4ty = atirados[i].pos_y+8;
                    p1ex = lista_inimigos[j].pos_x+1;
                    p1ey = lista_inimigos[j].pos_y+1;
                    p4ex = lista_inimigos[j].pos_x+lista_inimigos[j].tam_x;
                    p4ey = lista_inimigos[j].pos_y+lista_inimigos[j].tam_y;
                    if(p1tx <= p4ex && p4tx >= p1ex && p1ty <= p4ey && p4ty >= p1ey){
                        int top = max(p1ty, p1ey);
                        int bottom = min(p4ty, p4ey);
                        int left = max(p1tx, p1ex);
                        int right = min(p4tx, p4ex);
                        bool detectada = false;
                        al_lock_bitmap(inimigo_atirador[0], al_get_bitmap_format(inimigo_atirador[0]), ALLEGRO_LOCK_READONLY);
                        //CUIDAR PARA A CHEGADA DE NOVOS TIPOS DE INIMIGOS

                        ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], right-p1ex, top-p1ey);
                        if(color.a!=0)
                            detectada = true;

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], right-p1ex, bottom-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], left-p1ex, bottom-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], left-p1ex, top-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], ((right+left)/2)-p1ex, ((top+bottom)/2)-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        al_unlock_bitmap(inimigo_atirador[0]);
                        if (detectada){
                            lista_inimigos[j].pos_x = 999;
                            atirados[i].pos_x = 999;
                            qtd_tiros--;
                            pontuacao+=velocidade_inimigos+combo;
      
                            combo++;
                            max_combo = max(combo, max_combo);
                            if (lista_inimigos[j].torre >= 0)
                                lista_blocos[lista_inimigos[j].torre].frame_espinho = tempo_espinho;
                        }
                    }
                }
            }
        }
    }
}

void colisao_tiro_no_megaman(){
    for (int i = 0; i < max_tiros &&!gameover; i++){
        if (!atirados[i].megaman && atirados[i].pos_x < 999){
            int p1tx, p1ty, p4tx, p4ty;
            int p1ex, p1ey, p4ex, p4ey;
            ALLEGRO_BITMAP *frame_atual;

            if (estado == andando){
                p1ex = 60+19+1;
                p1ey = altura_andando+3+1;
                p4ex = 60+34;
                p4ey = altura_andando+40;
                if (atirando)
                    frame_atual = megaman_atirando[frame_andando];
                else
                    frame_atual = megaman[frame_andando];
            }
            else if(estado == pulando){
                p1ex = 60+18+1;
                p1ey = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+6+1;
                p4ex = 60+34;
                p4ey = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+48;
                if (atirando)
                    frame_atual = mega_pulo_atirando[frame_pulando];
                else
                    frame_atual = mega_pulo[frame_pulando];
            }
            else if(estado == dash){
                p1ex = 60+8+1;
                p1ey = altura_dash+8+1;
                p4ex = 60+47;
                p4ey = altura_dash+34;
                //frame_atual = mega_dash[frame_dash];
            }

            p1tx = atirados[i].pos_x+1;
            p1ty = atirados[i].pos_y+1;
            p4tx = atirados[i].pos_x+8;
            p4ty = atirados[i].pos_y+8;
            if(p1tx <= p4ex && p4tx >= p1ex && p1ty <= p4ey && p4ty >= p1ey){
                p1ex-=20;
                int top = max(p1ty, p1ey);
                int bottom = min(p4ty, p4ey);
                int left = max(p1tx, p1ex);
                int right = min(p4tx, p4ex);
                bool detectada = false;
                al_lock_bitmap(frame_atual, al_get_bitmap_format(frame_atual), ALLEGRO_LOCK_READONLY);
                //CUIDAR PARA A CHEGADA DE NOVOS TIPOS DE INIMIGOS

                ALLEGRO_COLOR color = al_get_pixel(frame_atual, right-p1ex, top-p1ey);
                if(color.a!=0)
                    detectada = true;

                if (!detectada){
                    ALLEGRO_COLOR color = al_get_pixel(frame_atual, right-p1ex, bottom-p1ey);
                    if(color.a!=0)
                        detectada = true;
                }

                if (!detectada){
                    ALLEGRO_COLOR color = al_get_pixel(frame_atual, left-p1ex, bottom-p1ey);
                    if(color.a!=0)
                        detectada = true;
                }

                if (!detectada){
                    ALLEGRO_COLOR color = al_get_pixel(frame_atual, left-p1ex, top-p1ey);
                    if(color.a!=0)
                        detectada = true;
                }

                if (!detectada){
                    ALLEGRO_COLOR color = al_get_pixel(frame_atual, ((right+left)/2)-p1ex, ((top+bottom)/2)-p1ey);
                    if(color.a!=0)
                        detectada = true;
                }

                al_unlock_bitmap(frame_atual);
                if (detectada)
                    gameover = true;
            }
        }
    }
}

void colisao_tiro_no_bloco(){
    for (int i = 0; i < max_tiros; i++){
        if (atirados[i].pos_x < 999){
            for (int j = 0; j < max_blocos; j++){
                if (lista_blocos[j].pos_x < 999){
                    int p1tx, p1ty, p4tx, p4ty;
                    int p1ex, p1ey, p4ex, p4ey;

                    p1tx = atirados[i].pos_x+1;
                    p1ty = atirados[i].pos_y+1;
                    p4tx = atirados[i].pos_x+10;
                    p4ty = atirados[i].pos_y+8;

                    if (lista_blocos[j].tipo == a){
                        p1ey = 118+1-7*(!lista_blocos[j].frame_espinho);
                        p4ey = 208;
                    }
                    else if (lista_blocos[j].tipo == b){
                        p1ey = 162+1-7*(!lista_blocos[j].frame_espinho);
                        p4ey = 208;
                    }
                    else if (lista_blocos[j].tipo == h){
                        p1ey = 0;
                        p4ey = 170+15*(!lista_blocos[j].frame_espinho);
                    }
                    else if (lista_blocos[j].tipo == f){
                        p1ey = 118+1-7*(!lista_blocos[j].frame_espinho);
                        p4ey = 140;
                    }
                    else if (lista_blocos[j].tipo == d){
                        p1ey = 140;
                        p4ey = 170+15*(!lista_blocos[j].frame_espinho);
                    }
                    p1ex = lista_blocos[j].pos_x+10;
                    p4ex = lista_blocos[j].pos_x+37;

                    if(p1tx <= p4ex && p4tx >= p1ex && p1ty <= p4ey && p4ty >= p1ey){
                        atirados[i].pos_x = 999;
                        if(atirados[i].megaman)
                            qtd_tiros--;
                        else{
                            combo_das_balas++;
                            
                        }
                    }
                }
            }
        }
    }
}

void colisao_blocos(){
    for (int i = 0; i < max_blocos && !gameover; i++){
        if (lista_blocos[i].pos_x < 999){
            int p1tx, p1ty, p4tx, p4ty;
            int p1ex, p1ey, p4ex, p4ey;

            if (estado == andando){
                p1tx = 60+19+1;
                p1ty = altura_andando+3+1;
                p4tx = 60+34;
                p4ty = altura_andando+40;
            }
            else if(estado == pulando){
                p1tx = 60+18+1;
                p1ty = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+6+1;
                p4tx = 60+34;
                p4ty = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+48;
            }
            else if(estado == dash){
                p1tx = 60+8+1;
                p1ty = altura_dash+8+1;
                p4tx = 60+47;
                p4ty = altura_dash+34;
            }

            if (lista_blocos[i].tipo == a){
                p1ey = 128+1;
                p4ey = 208;
            }
            else if (lista_blocos[i].tipo == b){
                p1ey = 169+1;
                p4ey = 208;
            }
            else if (lista_blocos[i].tipo == h){
                p1ey = 0;
                p4ey = 170+10*(!lista_blocos[i].frame_espinho);
            }
            else if (lista_blocos[i].tipo == f){
                p1ey = 118+1-7*(!lista_blocos[i].frame_espinho);
                p4ey = 140;
            }
            else if (lista_blocos[i].tipo == d){
                p1ey = 140;
                p4ey = 170+10*(!lista_blocos[i].frame_espinho);
            }

            p1ex = lista_blocos[i].pos_x+1;
            p4ex = lista_blocos[i].pos_x+37;

            if(p1tx <= p4ex && p4tx >= p1ex && p1ty <= p4ey && p4ty >= p1ey)
                    gameover = true;
        }
    }
}

void colisao_inimigos(){
    for (int i = 0; i < max_inimigos && !gameover; i++){
        if (lista_inimigos[i].pos_x < 999){
                    int p1tx, p1ty, p4tx, p4ty;
                    int p1ex, p1ey, p4ex, p4ey;

                    if (estado == andando){
                        p1tx = 60+19+1;
                        p1ty = altura_andando+3+1;
                        p4tx = 60+34;
                        p4ty = altura_andando+40;
                    }
                    else if(estado == pulando){
                        p1tx = 60+18+1;
                        p1ty = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+6+1;
                        p4tx = 60+34;
                        p4ty = altura_pulando-velocidade_inimigos*((altura_pulada*velocidade_pulo)/2)+48;
                    }
                    else if(estado == dash){
                        p1tx = 60+8+1;
                        p1ty = altura_dash+8+1;
                        p4tx = 60+47;
                        p4ty = altura_dash+34;
                    }
                    p1ex = lista_inimigos[i].pos_x+1;
                    p1ey = lista_inimigos[i].pos_y+1;
                    p4ex = lista_inimigos[i].pos_x+lista_inimigos[i].tam_x;
                    p4ey = lista_inimigos[i].pos_y+lista_inimigos[i].tam_y;
                    if(p1tx <= p4ex && p4tx >= p1ex && p1ty <= p4ey && p4ty >= p1ey){
                        int top = max(p1ty, p1ey);
                        int bottom = min(p4ty, p4ey);
                        int left = max(p1tx, p1ex);
                        int right = min(p4tx, p4ex);
                        bool detectada = false;
                        al_lock_bitmap(inimigo_atirador[0], al_get_bitmap_format(inimigo_atirador[0]), ALLEGRO_LOCK_READONLY);
                        //CUIDAR PARA A CHEGADA DE NOVOS TIPOS DE INIMIGOS

                        ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], right-p1ex, top-p1ey);
                        if(color.a!=0)
                            detectada = true;

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], right-p1ex, bottom-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], left-p1ex, bottom-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], left-p1ex, top-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        if (!detectada){
                            ALLEGRO_COLOR color = al_get_pixel(inimigo_atirador[0], ((right+left)/2)-p1ex, ((top+bottom)/2)-p1ey);
                            if(color.a!=0)
                                detectada = true;
                        }

                        al_unlock_bitmap(inimigo_atirador[0]);
                        if (detectada){
                            gameover = true;
                        }
                    }
        }
    }
}

void gerar_cenario(){
    for (int i = 0; i < LARGURA_TELA; i++){
        al_draw_bitmap(fundo, i*130-((tempo*velocidade_inimigos)%130), 0, 0);
        al_draw_bitmap(chao[0], i*16-((tempo*velocidade_inimigos)%16), 208, 0); //208
        al_draw_bitmap(chao[1], i*16-((tempo*velocidade_inimigos)%16), 214, 0);
        al_draw_bitmap(chao[1], i*16-((tempo*velocidade_inimigos)%16), 230, 0);
    }
}

void imprimir_pontuacao(){
    for (int i = 0; i < LARGURA_TELA/2; i++)
        al_draw_bitmap(painel, i*2, 0, 0);

        al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 60, 3, ALLEGRO_ALIGN_CENTRE, "Pontos: %d", pontuacao);
        al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 200, 3, ALLEGRO_ALIGN_CENTRE, "Combo atual: %d", combo);
        al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 360, 3, ALLEGRO_ALIGN_CENTRE, "Combo das Balas: %d", combo_das_balas);
        //if(winterbells)
            //al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 420, 3, ALLEGRO_ALIGN_CENTRE, "Winterbells: on");
        //else
          //  al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 420, 3, ALLEGRO_ALIGN_CENTRE, "Winterbells: off");
        int dificuldade = velocidade_inimigos+(6-(frequencia_obstaculos/40))+(4-frequencia_monstros)+(4-frequencia_tiros)*tiros;

        if (dificuldade > 9){
			nivel = 3;
            al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 548, 3, ALLEGRO_ALIGN_CENTRE, "Dificuldade: dificil");
		}
        else if(dificuldade > 5){
			nivel = 2;
            al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 548, 3, ALLEGRO_ALIGN_CENTRE, "Dificuldade: normal");
		}
        else{
			nivel = 1;
            al_draw_textf(fonte_pontos, al_map_rgb(0, 0, 0), 548, 3, ALLEGRO_ALIGN_CENTRE, "Dificuldade: facil");
		}
}

void reset(){
    for (int i = 0; i < max_tiros; i++)
        atirados[i].pos_x = 999;
    for (int i = 0; i < max_inimigos; i++)
        lista_inimigos[i].pos_x = 999;
    for (int i = 0; i < max_blocos; i++)
        lista_blocos[i].pos_x = 999;
    init=clock();
    qtd_tiros = 0;
    segurando = false;
    subindo = true;
    altura_pulada = 0;
    frame_pulando = 0;
    dash_tempo = 0;
    atirando_tempo = 0;
    atirando = false;
    tempo = 0;
    estado = andando;
    pontuacao = 0;
    combo = 0;
    combo_das_balas = 0;
}

void carregar_imagens(){
    megaman[0] = al_load_bitmap("parado.bmp");
    megaman[1] = al_load_bitmap("andando_1.bmp");
    megaman[2] = al_load_bitmap("andando_2.bmp");
    megaman[3] = al_load_bitmap("andando_3.bmp");
    megaman[4] = al_load_bitmap("andando_4.bmp");
    megaman[5] = al_load_bitmap("andando_5.bmp");
    megaman[6] = al_load_bitmap("andando_6.bmp");
    megaman[7] = al_load_bitmap("andando_7.bmp");
    megaman[8] = al_load_bitmap("andando_8.bmp");
    megaman[9] = al_load_bitmap("andando_9.bmp");
    megaman[10] = al_load_bitmap("andando_10.bmp");


    megaman_atirando[0] = al_load_bitmap("parado.bmp");
    megaman_atirando[1] = al_load_bitmap("andando_atirando_1.bmp");
    megaman_atirando[2] = al_load_bitmap("andando_atirando_2.bmp");
    megaman_atirando[3] = al_load_bitmap("andando_atirando_3.bmp");
    megaman_atirando[4] = al_load_bitmap("andando_atirando_4.bmp");
    megaman_atirando[5] = al_load_bitmap("andando_atirando_5.bmp");
    megaman_atirando[6] = al_load_bitmap("andando_atirando_6.bmp");
    megaman_atirando[7] = al_load_bitmap("andando_atirando_7.bmp");
    megaman_atirando[8] = al_load_bitmap("andando_atirando_8.bmp");
    megaman_atirando[9] = al_load_bitmap("andando_atirando_9.bmp");
    megaman_atirando[10] = al_load_bitmap("andando_atirando_10.bmp");

    mega_pulo[0] = al_load_bitmap("pulando_1.bmp");
    mega_pulo[1] = al_load_bitmap("pulando_2.bmp");
    mega_pulo[2] = al_load_bitmap("pulando_3.bmp");
    mega_pulo[3] = al_load_bitmap("pulando_4.bmp");
    mega_pulo[4] = al_load_bitmap("pulando_5.bmp");

    mega_pulo_atirando[0] = al_load_bitmap("pulando_atirando_1.bmp");
    mega_pulo_atirando[1] = al_load_bitmap("pulando_atirando_2.bmp");
    mega_pulo_atirando[2] = al_load_bitmap("pulando_atirando_3.bmp");
    mega_pulo_atirando[3] = al_load_bitmap("pulando_atirando_4.bmp");
    mega_pulo_atirando[4] = al_load_bitmap("pulando_atirando_5.bmp");
   
    chao[0] = al_load_bitmap("chao_1.bmp");
    chao[1] = al_load_bitmap("chao_2.bmp");
    fundo = al_load_bitmap("fundo.bmp");
    espinho = al_load_bitmap("espinho.bmp");
    espinho_menor = al_load_bitmap("espinho_menor.bmp");
    topo = al_load_bitmap("topo.bmp");
    painel = al_load_bitmap("painel_pontos.bmp");

    inimigo_atirador[0] = al_load_bitmap("atirador_1.bmp");
    inimigo_atirador[1] = al_load_bitmap("atirador_2.bmp");
    tiro = al_load_bitmap("tiro.bmp");
    tiro_atirador = al_load_bitmap("tiro_inimigo.bmp");

    fonte_over = al_load_font("OVER.ttf", 48, 0);
    fonte_pontos = al_load_font("PONTOS.ttf", 12, 0);
    the_end = al_load_bitmap("the_end.bmp");
}

int main(void){

	//Inicializa device
  	int dev = open("/dev/de2i150_altera", O_RDWR);
	setPontos(0, dev);// comunicacao com driver, enviar pontos para o display de 7 segmentos
	setCombo(0,dev);



    bool sair = false;
    if (!inicializar()){
        return -1;
    }

    carregar_imagens();
    reset();


    while (!sair)
    {
        if (gameover){
		piscaGameOver(dev); //comunicacao com a placa
		setPontos(0,dev);
		nivel = 0;
		level(dev);
		setCombo(0,dev);
            final=clock()-init;
            float segundos = (double)final / ((double)CLOCKS_PER_SEC);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            melhor_pontuacao = max(pontuacao, melhor_pontuacao);
            al_draw_bitmap(the_end, 0, 0, 0);

            
            al_draw_textf(fonte_pontos, al_map_rgb(255, 255, 255), LARGURA_TELA/2, ALTURA_TELA/2+15, ALLEGRO_ALIGN_CENTRE, "Sua pontuacao foi: %d", pontuacao);
            al_draw_textf(fonte_pontos, al_map_rgb(255, 255, 255), LARGURA_TELA/2, ALTURA_TELA/2+30, ALLEGRO_ALIGN_CENTRE, "Seu combo maximo foi: %d", max_combo);
            al_draw_textf(fonte_pontos, al_map_rgb(255, 255, 255), LARGURA_TELA/2, ALTURA_TELA/2+45, ALLEGRO_ALIGN_CENTRE, "Sua melhor pontuacao: %d", melhor_pontuacao);
            

            al_flip_display();
            ALLEGRO_EVENT evento;
            al_wait_for_event(fila_eventos, &evento);



            if (evento.type == ALLEGRO_EVENT_KEY_DOWN){
                gameover = false;
                reset();
            }

            if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
                sair = true;
            }
        }
        else{
            while (!al_is_event_queue_empty(fila_eventos)){
                ALLEGRO_EVENT evento;
                al_wait_for_event(fila_eventos, &evento);

                if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
                    sair = true;
                }
                else if (evento.type == ALLEGRO_EVENT_KEY_DOWN){
                    switch(evento.keyboard.keycode)
                    {
                    case ALLEGRO_KEY_W:
                        if(estado == andando || estado == dash){
                            if (estado == dash)
                                dash_tempo = 0;
                            estado = pulando;
                            frame_no_ar = tempo_no_ar*2/velocidade_inimigos;
                            segurando = true;
                        }
                        break;
                    case ALLEGRO_KEY_S:
                        if (atirando_tempo == 0 && qtd_tiros < max_tiros){
                            if (estado == dash){
                                estado = andando;
                                dash_tempo = 0;
                            }
                            atirando = true;
                            atirando_tempo = tempo_atirando;
                            qtd_tiros++;
                            criar_tiro(0, 0);
                        }
                        break;
                    case ALLEGRO_KEY_V:
                        if (velocidade_inimigos > 1)
                            velocidade_inimigos-=1;
                        break;
                    case ALLEGRO_KEY_B:
                        if (frequencia_obstaculos < 200)
                            frequencia_obstaculos+=40;
                        break;
                    case ALLEGRO_KEY_N:
                        if (frequencia_monstros < 3)
                            frequencia_monstros+=1;
                        break;
                    case ALLEGRO_KEY_F:
                        if (velocidade_inimigos < 3)
                            velocidade_inimigos+=1;
                        break;
                    case ALLEGRO_KEY_G:
                        if (frequencia_obstaculos > 120)
                            frequencia_obstaculos-=40;
                        break;
                    case ALLEGRO_KEY_H:
                        if (frequencia_monstros > 1)
                            frequencia_monstros-=1;
                        break;
                    case ALLEGRO_KEY_T:
                        if (frequencia_tiros > 1)
                            frequencia_tiros-=1;
                        break;
                    case ALLEGRO_KEY_Y:
                        if (frequencia_tiros < 3)
                            frequencia_tiros+=1;
                        break;
                    case ALLEGRO_KEY_E:
                        if (tiros)
                            tiros = false;
                        else
                            tiros = true;
                        break;
                    }
                }
                else if (evento.type == ALLEGRO_EVENT_KEY_UP){
                    switch(evento.keyboard.keycode)
                    {
                    case ALLEGRO_KEY_X:
                        segurando = false;
                        break;
                    }
                }
            }
            gerar_cenario();

            if (estado == andando){
                frame_andando = (tempo/velocidade_animacao)%10+1;
                if (atirando)
                    al_draw_bitmap(megaman_atirando[frame_andando], 60, altura_andando, 0);
                else
                    al_draw_bitmap(megaman[frame_andando], 60, altura_andando, 0);
            }
            else if (estado == pulando){
                int add = velocidade_inimigos*((altura_pulada*velocidade_pulo)/2);
                if (atirando)
                    al_draw_bitmap(mega_pulo_atirando[frame_pulando], 60, altura_pulando-add, 0);
                else
                    al_draw_bitmap(mega_pulo[frame_pulando], 60, altura_pulando-add, 0);
                if (subindo && add < altura_pulo && segurando){
                    altura_pulada++;
                    if (altura_pulada == 0)
                        frame_pulando++;
                    else if (altura_pulada == 3)
                        frame_pulando++;
                    else if (altura_pulada == 7)
                        frame_pulando++;
                    else if (altura_pulada == 11)
                        frame_pulando++;
                }
                else if(segurando && frame_no_ar > 0){
                    frame_no_ar--;
                }
                else if (add >= altura_pulo || (subindo && !segurando)){
                    subindo = false;
                    altura_pulada--;
                }
                else if (!subindo && add > 0){
                    altura_pulada--;
                }
                else if (!subindo && add <= 0){
                    subindo = true;
                    altura_pulada = 0;
                    estado = andando;
                    frame_pulando = 0;
                    frame_no_ar = 0;
                }
            }
           
            criar_obstaculos();
            imprimir_tiro();
            imprimir_blocos();
            imprimir_inimigo();
            imprimir_pontuacao();
			setPontos(pontuacao,dev);// comunicacao com driver, enviar pontos para o display de 7 segmentos
			level(dev); // leds verdes indicam o nivel
			setCombo(combo,dev); //combo atual no display
            colisao_tiro_no_inimigo();
            colisao_tiro_no_megaman();
            colisao_tiro_no_bloco();
            colisao_inimigos();
            colisao_blocos();
            atualizar_blocos();
            atualizar_inimigos();
            atualizar_tiro();
            if (atirando_tempo > 0){
                atirando_tempo--;
                if (atirando_tempo == 0)
                    atirando = false;
            }
            tempo++;
            if (tempo < 0)
                tempo = 0;
        }
        al_flip_display();
        al_rest(fps);
    }

    al_destroy_event_queue(fila_eventos);
    al_destroy_display(janela);

    return 0;
}

bool inicializar(){
    if (!al_init()){
        fprintf(stderr, "Falha ao inicializar Allegro.\n");
        return false;
    }

    janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!janela){
        fprintf(stderr, "Falha ao criar janela.\n");
        return false;
    }


    fila_eventos = al_create_event_queue();
    if (!fila_eventos){
        fprintf(stderr, "Falha ao criar fila de eventos.\n");
        al_destroy_display(janela);
        return false;
    }


    if(!al_init_image_addon()){
        fprintf(stderr, "Falha ao iniciar addon allegro_image.\n");
        return false;
    }

    al_init_font_addon();

    if (!al_init_ttf_addon()){
        fprintf(stderr, "Falha ao inicializar add-on allegro_ttf.\n");
        return -1;
    }

    if (!al_install_keyboard())
    {
        fprintf(stderr, "Falha ao inicializar o teclado.\n");
        return false;
    }

    al_set_window_title(janela, "IHS - Sunset Riders");
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());

    al_register_event_source(fila_eventos, al_get_display_event_source(janela));

    return true;
}

