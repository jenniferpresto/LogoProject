//
//  ParticleSystem.cpp
//  LogoProject
//
//  Created by Jennifer on 9/8/15.
//
//

#include "ParticleSystem.h"

ParticleSystemRef ParticleSystem::create(float &posArray)
{
    ParticleSystemRef ref(new ParticleSystem());
    ref->setup(posArray);
    return ref;
}

ParticleSystem::ParticleSystem()
{}

ParticleSystem::~ParticleSystem()
{
    std::cout << "Calling particle system destructor" << std::endl;
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mParticleBufferA);
    glDeleteBuffers(1, &mParticleBufferB);
    mPotionTex->unbind();
    mParticleTex->unbind();
}

void ParticleSystem::setup(float &posArray)
{
    //  create and bind the vao
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    mLastMousePos = ci::vec2(0.0f, 0.0f);
    
    loadTextures();
    
    //  array of values
    GLfloat positionData[logo::NUM_PARTICLES * 7]; // two slots for position, two for velocity, three for color
    //GLfloat newPositionData[mMaxNewPositions * 2];
    
    for (int i = 0; i < logo::NUM_PARTICLES; i++) {
        //  position is completely random
        float randNum = ci::Rand::randFloat(-1.0f, 1.0f);
        float randNum2 = ci::Rand::randFloat(-1.0f, 1.0f);
        //  velocity is normalized vector
        float randNum3 = ci::Rand::randFloat(0.0f, M_PI * 2.0f);
        
        positionData[(i*7) + 0] = randNum;              // pos.x
        positionData[(i*7) + 1] = randNum2;             // pos.y
        
        positionData[(i*7) + 2] = cos(randNum3) * 0.03; // vel.x
        positionData[(i*7) + 3] = sin(randNum3) * 0.03; // vel.y
        
        positionData[(i*7) + 4] = ci::Rand::randFloat();  // col.r
        positionData[(i*7) + 5] = ci::Rand::randFloat();  // col.g
        positionData[(i*7) + 6] = ci::Rand::randFloat();  // col.b
    }
    
    std::cout << "Sizeof positionData: " << sizeof(positionData) << std::endl;
    
    //  create two buffers to ping-pong back and forth with position data
    glGenBuffers(1, &mParticleBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &mParticleBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STREAM_DRAW); // don't initialize immediately
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //  load the external files
    std::string vertexShaderSrcString = loadShaderSource("vertex.glsl");
    std::string fragmentShaderSrcString = loadShaderSource("frag.glsl");
    
    //  convert to const char *
    const char * vertexShaderSrc = vertexShaderSrcString.c_str();
    const char * fragShaderSrc = fragmentShaderSrcString.c_str();
    
    //  create the shader program
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragShaderSrc);
    mShaderProgram = glCreateProgram();
    
    std::cout << "vertexShader: " << vertexShader << std::endl;
    std::cout << "fragmentShader: " << fragmentShader << std::endl;
    std::cout << "mShaderProgram: " << mShaderProgram << std::endl;
    glAttachShader(mShaderProgram, vertexShader);
    glAttachShader(mShaderProgram, fragmentShader);

    // before linking program, specify which output attributes we want to capture into a buffer
    const GLchar * feedbackVaryings[3] = {"outPos", "outVel", "outCol"};
    glTransformFeedbackVaryings(mShaderProgram, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glBindFragDataLocation(mShaderProgram, 0, "outColor");

    // link and activate the program
    glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    
    mPosAttrib = glGetAttribLocation(mShaderProgram, "inPos");
    mVelAttrib = glGetAttribLocation(mShaderProgram, "inVel");
    mColAttrib = glGetAttribLocation(mShaderProgram, "inCol");

    mMousePosUniform = glGetUniformLocation(mShaderProgram, "mousePos");
    mNewPosUniform = glGetUniformLocation(mShaderProgram, "newPositions");
    mParticleTexUniform = glGetUniformLocation(mShaderProgram, "ParticleTex");
    mPotionTexUniform = glGetUniformLocation(mShaderProgram, "PotionTex");
    
    std::cout << "Max uniform locations: " << GL_MAX_UNIFORM_LOCATIONS << std::endl;
  
    std::cout << "mPosAttrib: " << mPosAttrib << std::endl;
    std::cout << "mVelAttrib: " << mVelAttrib << std::endl;
    std::cout << "mColAttrib: " << mColAttrib << std::endl;

    std::cout << "mMousePosUniform:" << mMousePosUniform << std::endl;
    std::cout << "mNewPosUniform: " << mNewPosUniform << std::endl;
    std::cout << "mParticleTexUniform: " << mParticleTexUniform << std::endl;
    std::cout << "mPotionTexUniform: " << mPotionTexUniform << std::endl;
    
    
    mPosArrayPointer = &posArray;
    
    //std::cout << "RandFloat on 1.1, 1.5)" << getRandomFloat(ci::vec2(1.1, 1.5)) << std::endl;
    //std::cout << "RandFloat on 1.5, 1.7)" << getRandomFloat(ci::vec2(1.5, 1.7)) << std::endl;
    //std::cout << "RandFloat on 1.9, 1.5)" << getRandomFloat(ci::vec2(1.9, 1.5)) << std::endl;
    //std::cout << "RandFloat on 0.9, 0.3)" << getRandomFloat(ci::vec2(0.9, 0.3)) << std::endl;
    
}

//******************************************
//  nothing happens in update
//******************************************
void ParticleSystem::update()
{
    
}

//******************************************
//  temp method for debugging
//******************************************
void ParticleSystem::updateMouse(ci::vec2 pos)
{
    mLastMousePos = pos;
}

//******************************************
//  draw function; main updates happen here
//******************************************
void ParticleSystem::draw()
{
    //glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    
    //  make a copy of the array from the main app
    float testArray[logo::NUM_NEW_POSITIONS * 2];
    for (int i = 0; i < logo::NUM_NEW_POSITIONS * 2; i++) {
        testArray[i] = mPosArrayPointer[i];
    }

    //  pass mouse position
    //float mousePos[2] = {float(mLastMousePos.x), float(mLastMousePos.y)};
    //glUniform2fv(mMousePosUniform, 1, mousePos);
    
    //  pass in the array of new positions
    glUniform2fv(mNewPosUniform, logo::NUM_NEW_POSITIONS, testArray);
    
    
    //  disable the rasterizer
    glEnable(GL_RASTERIZER_DISCARD);
    
    //  specify the source buffer
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    
    
    //  specify target buffer
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBufferB);
    
    //  do transform feedback
    glBeginTransformFeedback(GL_POINTS);
    
    glDrawArrays(GL_POINTS, 0, logo::NUM_PARTICLES);
    glEndTransformFeedback();
    
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
    
    glFlush();

    std::swap(mParticleBufferA, mParticleBufferB);
    
    //  draw the particles
    glDisable(GL_RASTERIZER_DISCARD);
    
    //  Cinder openGL calls to render textures as points
    // glUniform1i(mParticleTexUniform, 0);
    ci::gl::ScopedTextureBind texScope( mParticleTex , 0);
    ci::gl::ScopedTextureBind texScope2(mPotionTex, 1);
    glUniform1i(mPotionTexUniform, 1);
    
    ci::gl::ScopedState	stateScope( GL_PROGRAM_POINT_SIZE, true );
    ci::gl::ScopedBlend blendScope( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    
    glDrawArrays(GL_POINTS, 0, logo::NUM_PARTICLES);
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
}

//******************************************
//  load shader from plain text
//******************************************
std::string ParticleSystem::loadShaderSource(std::string path)
{
    std::ifstream in(ci::app::getAssetPath(path).string());
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    return contents;
}

//******************************************
//  load textures
//******************************************
void ParticleSystem::loadTextures()
{
    ci::gl::Texture::Format textureFormat;
    textureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mParticleTex = ci::gl::Texture::create( ci::loadImage( ci::app::loadAsset( "smoke_blur.png" ) ), textureFormat );
    
    ci::gl::Texture::Format textureFormat2;
    textureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mPotionTex = ci::gl::Texture::create(ci::loadImage(ci::app::loadAsset("potionBubbles.png")), textureFormat);
    
    std::cout << "mParticleTex id: " << mParticleTex->getId() << std::endl;
    std::cout << "mParticleTex id: " << mPotionTex->getId() << std::endl;
    
}


//******************************************
// create shader and print out debugging info
//******************************************
GLuint ParticleSystem::createShader(GLenum type, const GLchar* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    //std::cout << src << std::endl;
    glCompileShader(shader);
    
    // debug shader
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    char buffer[512];
    std::cout << "Shader status: (0 bad, 1 good): " << status << std::endl;
    if (status == 0) {
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        std::cout << "Shader compile log: " << buffer << std::endl;
    }
    
    return shader;
}

//******************************************
// functions from shaders for testing
//******************************************

float ParticleSystem::getRandomFloat(ci::vec2 currentPos) {
    //return fract(sin(dot(currentPos.xy, ci::vec2(12.9898, 78.233))) * 43758.5453);
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(ci::vec2(currentPos.x, currentPos.y), ci::vec2(a, b));
    //float sn = mod(dt, 3.14);
    float sn = dt - 3.14 * floor(dt/3.14);
    return (sin(sn) * c) - floor(sin(sn) * c);
}

float ParticleSystem::mapFloat(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{
    const float Epsilon = 0.0000001;
    if (fabs(inputMin - inputMax) < Epsilon) {
        return outputMin;
    }
    
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
    // clamp result
    if (outVal > outputMax) {
        outVal = outputMax;
    }
    if (outVal < outputMin) {
        outVal = outputMin;
    }
    return outVal;
}
