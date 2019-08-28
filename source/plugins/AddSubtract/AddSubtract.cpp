#include "FFGL.h"
#include "FFGLLib.h"

#include "AddSubtract.h"
#include "../../lib/ffgl/utilities/utilities.h"

#define FFPARAM_BrightnessR  (0)
#define FFPARAM_BrightnessG	 (1)
#define FFPARAM_BrightnessB	 (2)
#define FFPARAM_OSC_Text    (3)
#define FFPARAM_OSC_Size    (4)
////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
	AddSubtract::CreateInstance,		// Create method
	"PROT201908",								// Plugin unique ID
	"P Recv Osc Text",					// Plugin name
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
                                                        
uniform float float1;
void main()
{
//    vec4 color = texture2D(inputTexture,gl_TexCoord[0].st);
//    if (color.a > 0.0) //unpremultiply
//        color.rgb /= color.a;
//    color.rgb = color.rgb + brightness;
//    color.rgb *= color.a; //premultiply
//    color.r = sin(float1);
//    color.rgb = vec3(sin(float1));
//    color.a = 1.0;
    
    vec4 color = vec4(1.0,0.0,0.0,1.0);
	gl_FragColor  =  color;
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

    SetParamInfo(FFPARAM_OSC_Text, "osc text", FF_TYPE_TEXT, "hello arena");
    oscTextStr = "hello arena";
    
//    oscTextSize = 10.0f;
    SetParamInfo(FFPARAM_OSC_Size, "osc text size", FF_TYPE_STANDARD, 1.0f/oscTextSize);
    
}

AddSubtract::~AddSubtract()
{
	
}

FFResult AddSubtract::InitGL(const FFGLViewportStruct *vp)
{

	m_initResources = 0;

    float1 = 0.0f;
//    oscTextSize = oscTextStr.size();//

	//initialize gl shader
	m_shader.Compile(vertexShaderCode,fragmentShaderCode);

	//activate our shader
	m_shader.BindShader();

	//to assign values to parameters in the shader, we have to lookup
	//the "location" of each value.. then call one of the glUniform* methods
	//to assign a value
	m_inputTextureLocation = m_shader.FindUniform("inputTexture");
	m_BrightnessLocation = m_shader.FindUniform("brightness");
    float1Loc = m_shader.FindUniform("float1");
    
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

    
//    oscTextSize = oscTextStr.size();
    oscTextSize += 1.0f;
    float1 = getTicks();
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
	
    glUniform1f(float1Loc, float1);
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
    case FFPARAM_OSC_Size:
        retValue = 1.0/oscTextSize;
        return retValue;
	default:
		return retValue;
	}
}

//char* AddSubtract::GetTextParameter(unsigned int dwIndex)
//{
//    
//    char* retValue;
//    switch (dwIndex) {
//        case FFPARAM_OSC_Text:
//            retValue = const_cast<char*>(oscTextStr.c_str());
//            break;
//            
//        default:
//            return (char *)FF_FAIL;
//    }
//    return retValue;
//}
//
//FFResult AddSubtract::SetTextParameter(unsigned int dwIndex, const char *value){
//    switch (dwIndex)
//    {
//
//        case FFPARAM_OSC_Text:
//            oscTextStr.clear();
//            oscTextStr = value;
//            break;
//        default:
//            return FF_FAIL;
//    }
//    
//    return FF_SUCCESS;
//}

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
//        case FFPARAM_OSC_Size:
//            oscTextSize = 1.0 / value;
//            break;
	default:
		return FF_FAIL;
	}

	return FF_SUCCESS;
}
