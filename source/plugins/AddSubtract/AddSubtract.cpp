#include "FFGL.h"
#include "FFGLLib.h"
#include <math.h>
#include "AddSubtract.h"
#include "../../lib/ffgl/utilities/utilities.h"
#include "MyUtils.h"


#define FFPARAM_BrightnessR  (0)
#define FFPARAM_BrightnessG	 (1)
#define FFPARAM_BrightnessB     (2)
#define FFPARAM_text_data     (3)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
	AddSubtract::CreateInstance,		// Create method
	"RE11",								// Plugin unique ID
	"AddSub Example",					// Plugin name
	1,						   			// API major version number 													
	500,								// API minor version number
	1,									// Plugin major version number
	000,								// Plugin minor version number
	FF_EFFECT,							// Plugin type
	"Add and Subtract colours",			// Plugin description
	"Resolume FFGL Example"				// About
);

static const std::string vertexShaderCode = STRINGIFY(
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}
);


static const std::string fragmentShaderCode = STRINGIFY(
uniform sampler2D inputTexture;
uniform vec3 brightness;
uniform float width;
uniform float height;
uniform float ticks;
uniform float inputArray[256]; // 256 size()
uniform float probeFFT;
void main()
{
    // name convert ---------------
    vec4 fragColor;
    float iTime = ticks/1000.0;
    vec2 iResolution = vec2(width,height);
    vec2 fragCoord = vec2(gl_FragCoord.x,iResolution.y - gl_FragCoord.y);

    vec2 uv = fragCoord.xy/iResolution.xy;

    vec3 col;
    
    float num = 256.0;

    vec2 uv1 = uv;
    uv1.x *= num;
    float f = fract(uv1.x);
    int index1 = int(floor(uv1.x));
    if(uv.y < inputArray[index1]){
        col = vec3(f);

    }

//        col = vec3(probeFFT,0.0,0.0);
//    col += vec3(inputArray[index1],0.0,0.0);
    // ---------------
    fragColor = vec4(col,1.0);
    
    
    // finish ---------------
    gl_FragColor = fragColor;

}
);

AddSubtract::AddSubtract()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_inputTextureLocation(-1),
 m_BrightnessLocation(-1)
{
	// Input properties
	SetMinInputs(1);
	SetMaxInputs(1);

	// Parameters
	SetParamInfo(FFPARAM_BrightnessR, "R", FF_TYPE_RED, 0.5f);
	m_BrightnessR = 0.5f;

	SetParamInfo(FFPARAM_BrightnessG, "G", FF_TYPE_GREEN, 0.5f);
	m_BrightnessG = 0.5f;
	
	SetParamInfo(FFPARAM_BrightnessB, "B", FF_TYPE_BLUE, 0.5f);
	m_BrightnessB = 0.5f;
    
    rawOscTextData = "hello";
    SetParamInfo(FFPARAM_text_data, "osc text data", FF_TYPE_TEXT, rawOscTextData.c_str());
    

}

AddSubtract::~AddSubtract()
{
	
}

FFResult AddSubtract::InitGL(const FFGLViewportStruct *vp)
{

	m_initResources = 0;


    ticks = 0.0f;
	//initialize gl shader
	m_shader.Compile(vertexShaderCode,fragmentShaderCode);

	//activate our shader
	m_shader.BindShader();

	//to assign values to parameters in the shader, we have to lookup
	//the "location" of each value.. then call one of the glUniform* methods
	//to assign a value
	m_inputTextureLocation = m_shader.FindUniform("inputTexture");
	m_BrightnessLocation = m_shader.FindUniform("brightness");

    inputArrayLoc = m_shader.FindUniform("inputArray");
    m_TicksLocation = m_shader.FindUniform("ticks");
    
    m_WidthLocation = m_shader.FindUniform("width");
    m_HeightLocation = m_shader.FindUniform("height");
    
    probeFFTLoc = m_shader.FindUniform("probeFFT");
	//the 0 means that the 'inputTexture' in
	//the shader will use the texture bound to GL texture unit 0
	glUniform1i(m_inputTextureLocation, 0);
	
	m_shader.UnbindShader();

  return FF_SUCCESS;
}

FFResult AddSubtract::DeInitGL()
{
  m_shader.FreeGLResources();


  return FF_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////



FFResult AddSubtract::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	if (pGL->numInputTextures<1)
		return FF_FAIL;

	if (pGL->inputTextures[0]==NULL)
		return FF_FAIL;

    
    ticks = getTicks();
    
    
    // processing gotten osc text data
    std::vector<float> oscDataInFloatVec = MyConvertStingToFloatVector(rawOscTextData);
    
    // make use array size is right with default value 0
//    oscDataInFloatVec.resize(kArraySize);
    
    
    GLfloat data[256];
    for (int i = 0; i < 256; i++) {
//        data[i] = 0.003 * float(i);
        data[i] = oscDataInFloatVec[i];
    }
    
    
    
    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    
	//activate our shader
	m_shader.BindShader();

	FFGLTextureStruct &Texture = *(pGL->inputTextures[0]);

	//get the max s,t that correspond to the 
	//width,height of the used portion of the allocated texture space
	FFGLTexCoords maxCoords = GetMaxGLTexCoords(Texture);

    
	//assign the Brightness
	glUniform3f(m_BrightnessLocation,
				-1.0f + (m_BrightnessR * 2.0f),
				-1.0f + (m_BrightnessG * 2.0f),
				-1.0f + (m_BrightnessB * 2.0f)
				);
    // assign ticks in millisecond
    glUniform1f(m_TicksLocation,ticks);
    

    // assign width and height
    glUniform1f(m_WidthLocation, (float)viewport[2]);
    glUniform1f(m_HeightLocation, (float)viewport[3]);
    
    glUniform1fv(inputArrayLoc, kArraySize, data);

    glUniform1f(probeFFTLoc, data[200]);
	//activate texture unit 1 and bind the input texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture.Handle);
    
	//draw the quad that will be painted by the shader/textures
	//note that we are sending texture coordinates to texture unit 1..
	//the vertex shader and fragment shader refer to this when querying for
	//texture coordinates of the inputTexture
	glBegin(GL_QUADS);

	//lower left
	glMultiTexCoord2f(GL_TEXTURE0, 0,0);
	glVertex2f(-1,-1);

	//upper left
	glMultiTexCoord2f(GL_TEXTURE0, 0, maxCoords.t);
	glVertex2f(-1,1);

	//upper right
	glMultiTexCoord2f(GL_TEXTURE0, maxCoords.s, maxCoords.t);
	glVertex2f(1,1);

	//lower right
	glMultiTexCoord2f(GL_TEXTURE0, maxCoords.s, 0);
	glVertex2f(1,-1);
	glEnd();

	//unbind the input texture
	glBindTexture(GL_TEXTURE_2D,0);


	//unbind the shader
	m_shader.UnbindShader();

	return FF_SUCCESS;
}

float AddSubtract::GetFloatParameter(unsigned int dwIndex)
{
	float retValue = 0.0;

	switch (dwIndex)
	{
	case FFPARAM_BrightnessR:
		retValue = m_BrightnessR;
		return retValue;
	case FFPARAM_BrightnessG:
		retValue = m_BrightnessG;
		return retValue;
	case FFPARAM_BrightnessB:
		retValue = m_BrightnessB;
		return retValue;

	default:
		return retValue;
	}
}

FFResult AddSubtract::SetFloatParameter(unsigned int dwIndex, float value)
{
	switch (dwIndex)
	{
	case FFPARAM_BrightnessR:
		m_BrightnessR = value;
		break;
	case FFPARAM_BrightnessG:
		m_BrightnessG = value;
		break;
	case FFPARAM_BrightnessB:
		m_BrightnessB = value;
		break;
	default:
		return FF_FAIL;
	}

	return FF_SUCCESS;
}

char* AddSubtract::GetTextParameter(unsigned int dwIndex)
{

    char* retValue;
    switch (dwIndex) {
        case FFPARAM_text_data:
            retValue = const_cast<char*>(rawOscTextData.c_str());
            break;

        default:
            return (char *)FF_FAIL;
    }
    return retValue;
}

FFResult AddSubtract::SetTextParameter(unsigned int dwIndex, const char *value){
    switch (dwIndex)
    {

        case FFPARAM_text_data:
            rawOscTextData = value;
            break;
        default:
            return FF_FAIL;
    }

    return FF_SUCCESS;
}
