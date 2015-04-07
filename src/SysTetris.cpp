/* 
 * File:   Tetris.cpp
 * Author: cecco
 * 
 * Created on 24 marzo 2015, 17.36
 */
#include <stdlib.h>
#include <time.h> 

#include "SysGame.h"
#include "SysTetris.h"
#include "SysTetris/Brick.h"
#include "SysTetris/Eliminated3dCube.h"


SysTetris::SysTetris() {
    
    //init field
    for(int i=0; i<FIELD_H; i++)
        for(int j=0; j<FIELD_W; j++)
            field[i][j] = NULL;
    
    srand(time(0));
    
    currentBrick = newBrick();
    nextBrick = newBrick();
}


SysTetris::~SysTetris() {
    if(currentBrick != NULL) {
        currentBrick->delete3dCubes();
        delete currentBrick;
    }    
        
    if(nextBrick != NULL) {
        nextBrick->delete3dCubes();
        delete nextBrick;
    }
    
    for(int i=0; i<FIELD_H; i++)
        for(int j=0; j<FIELD_W; j++)
            if(field[i][j] != NULL) delete field[i][j];
        
    for(Eliminated3dCube* elem : freeBrick3dCube)
        delete elem;
    
    freeBrick3dCube.clear();
}

Brick3dCube* SysTetris::getField(int r, int c) {
    return field[r][c];
}

void SysTetris::update(float dt) {
        
    float speed = FallSpeed+AdditionFallSpeed+ score/500;
    speed >= MaxFallSpeed? speed = MaxFallSpeed : speed; 
    FallTime += speed*dt;
    
    MinTimeMove >0 ? MinTimeMove -= MoveSpeed*dt : MinTimeMove;
    
    int xtmp = currentBrick->getX();
    int ytmp = currentBrick->getY();
    
    if(MinTimeMove<= 0) {
        if( Left && currentBrick->checkPos( xtmp-1, ytmp,field)) {
                currentBrick->setBodyCubesSpeed(Brick::LATERAL);
                currentBrick->setX(xtmp-1);
                currentBrick->setY(ytmp);
        } else if (Right && currentBrick->checkPos( xtmp+1, ytmp,field)) {
                currentBrick->setBodyCubesSpeed(Brick::LATERAL);
                currentBrick->setX(xtmp+1);
                currentBrick->setY(ytmp);
        }
        MinTimeMove=10;
    }

    if(FallTime >=10) {        
        if(currentBrick->checkPos( xtmp, ytmp+1,field)) {
            currentBrick->setBodyCubesSpeed(Brick::VERTICAL);
            currentBrick->setX(xtmp);
            currentBrick->setY(ytmp+1);
        } else {
            currentBrick->setBodyCubesSpeed(Brick::ROWFALL);
            score += 10;
            currentBrick->saveOnField(field);
            checkFullLines();
            
            createBrick();
        }    
        FallTime=0;
    }

    nextBrickRotation += 40*dt;
           
    
    for(int i=0 ; i<FIELD_H ; i++) {
        for(int j=0; j<FIELD_W; j++) {
            if(field[i][j] != NULL) {
                field[i][j]->setXY(j,i);
                field[i][j]->updatePosition(dt);
 
            }
        }
    }
    
    for( std::list<Eliminated3dCube*>::iterator elem = freeBrick3dCube.begin() ;
         elem != freeBrick3dCube.end() ; elem++) {
        
        if(!(*elem)->updatePosition(dt)) {
            delete (*elem);
            elem =freeBrick3dCube.erase(elem);
        }
    }
    
    currentBrick->update(dt); 

}

void SysTetris::draw() {
   
    glLoadIdentity();

    //position of the field
    glTranslatef(XField, YField, ZField);
    glRotatef(XFieldRot, 1.0, 0, 0);
    glRotatef(YFieldRot, 0, 1.0, 0);
    glRotatef(ZFieldRot, 0, 0, 1.0);
       
    //move in the top left edge of the field
    glTranslatef(-(l*FIELD_W-1), l*FIELD_H, 0);
    
    glPushMatrix();

    int x = currentBrick->getX();
    int y = currentBrick->getY();
    
    //draw current brick
    for(int i=0; i<4; i++) {
        Brick3dCube* elem = currentBrick->get3Dcubes()[i];
        
        glPopMatrix();
        glPushMatrix();
        
        glTranslatef(2*l*elem->getXreal(), -2*l*elem->getYreal(), 0);
        drawColoredCube(l, colorFromId(elem->getID()));
    }
    
    glPopMatrix();
    
    
    //draw cubes on field
    for(int i=0; i<FIELD_H; i++) 
        for(int j=0; j<FIELD_W; j++) { 
            Brick3dCube* tmp = field[i][j];
            
            if( tmp != NULL) {
                glPushMatrix();
                glTranslatef(2*l*tmp->getXreal(), -2*l*tmp->getYreal(), 0.0f);
                drawColoredCube(l, colorFromId(field[i][j]->getID()));
                glPopMatrix();
            }
        }
    
    glPushMatrix();
    
    //draw  next brick
    glTranslatef(2*l*(FIELD_W-2), -l*4, +10); //posizione
    glTranslatef(2*l*2, -2*l, 0.0f);          //accentra
    glRotatef(nextBrickRotation, 1, 1, 1);    //ruota
    glTranslatef(-2*l*2, +2*l, 0.0f);         //decentra
    
    if(nextBrick != NULL) {
        for(int i=0; i<4; i++) {
            for(int j=0; j<4; j++) {
                if(nextBrick->getBodyValue(i,j) != 0)
                    drawColoredCube(l, colorFromId(nextBrick->getID()));
                glTranslatef(2*l, 0.0f, 0.0f);
            }
            glTranslatef(-(2*l*4), -(2*l), 0.0f);
        }
    }
    
    //disegna cubi eliminati
    for(Eliminated3dCube* elem : freeBrick3dCube) {
        glPopMatrix();
        glPushMatrix();
        
        glTranslatef(elem->getXreal(), elem->getYreal(), elem->getZreal());
        glRotatef(elem->getRotation(), 1.0f, 0.0f, 0.0f);
        drawColoredCube(l, colorFromId(elem->getID()));
    }    
    
    glPopMatrix();
}

void SysTetris::fallBrick() {
    currentBrick->setBodyCubesSpeed(Brick::VERTICAL);
    
    int xtmp = currentBrick->getX();
    int ytmp = currentBrick->getY()+1;
    
    while(ytmp <FIELD_H && currentBrick->checkPos( xtmp, ytmp,field)) {
        currentBrick->setX(xtmp);
        currentBrick->setY(ytmp);
        ytmp++;
    }

    score+=10;
    currentBrick->saveOnField(field);
    checkFullLines();
        
    createBrick();
}

void SysTetris::rotateBrick() {
    
    currentBrick->rotatedCheckPos(field);
}

void SysTetris::createBrick() {

    if(currentBrick != NULL)
        delete currentBrick;
    
    currentBrick = nextBrick;
    nextBrick = newBrick();
    
    if(!currentBrick->checkPos(currentBrick->getX(),currentBrick->getY(),field))
        gameOver=true;
}


Brick * SysTetris::newBrick() {

    Brick *nBrick;
     
    switch (rand() % 7+1) {
        case 1:
            nBrick = new Quad(5);
            break;
        case 2:
            nBrick = new Bar(5);
            break;
        case 3:
            nBrick = new TBar(5);
            break;            
        case 4:
            nBrick = new LBar(5);
            break;            
        case 5:
            nBrick = new RLBar(5);
            break;
        case 6:
            nBrick = new ZBar(5);
            break;
        case 7:
            nBrick = new RZBar(5);
            break;            
    }
    
    return nBrick;
}

COLOR SysTetris::colorFromId(int id) {
    switch (id) {
        case 1:
            return { 1.0f, 1.0f, 0.0f, 0.2f};
        case 2:
            return { 0.0f, 1.0f, 0.9f, 0.2f};
        case 3:
            return { 1.0f, 0.0f, 1.0f, 0.2f};           
        case 4:
            return { 0.0f, 0.0f, 1.0f, 0.2f};            
        case 5:
            return { 1.0f, 0.5f, 0.0f, 0.2f};
        case 6:
            return { 0.0f, 1.0f, 0.0f, 0.2f};
        case 7:
            return { 1.0f, 0.0f, 0.0f, 0.2f};
        default:
            return { 1.0f, 1.0f, 1.0f, 0.2f};
    }
}

void SysTetris::checkFullLines() {   
    int toRemove[6];
    int k=0;
    
    for(int i=0; i< FIELD_H; i++) {
        bool lineaPiena =true;
        for(int j=0;lineaPiena && j<FIELD_W; j++) {
            if(field[i][j] == NULL)
                lineaPiena=false;
        }
        if(lineaPiena) {
            toRemove[k] = i;
            k = k+1;
        }
    }

    score += 100*k;
    
    for(int i=0; i<k; i++) {
        //save free bricks
        for(int j=0; j<FIELD_W; j++) {
            Brick3dCube *src= field[toRemove[i]][j];
            
            float x = src->getX()*2*l;
            float y = -src->getY()*2*l;
            
            Eliminated3dCube *tmp= new Eliminated3dCube(src->getID(), x,y,0,l);
            delete src;
            
            freeBrick3dCube.push_front(tmp);
        }
        
        removeLine(toRemove[i]);
    }
}

void SysTetris::removeLine(int n) {
    if(n<1)
        return;
    
    copyLine(n-1,n);
    removeLine(n-1);
}

bool SysTetris::copyLine(int s, int d) {
    if(s<0 || s>=FIELD_H || d<0 || d>=FIELD_H)
        return false;
    
    //MEMORY LEAK
    for(int i=0; i<FIELD_W; i++) {
        field[d][i] = field[s][i];
    }
    
    return true;
}

void SysTetris::addFallSpeed(bool s) {

    if(s)
        AdditionFallSpeed = 70;
    else
        AdditionFallSpeed =0;
}
