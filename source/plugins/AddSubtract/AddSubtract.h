#pragma once

#include "FFGLShader.h"
#include "FFGLPluginSDK.h"
#include <string>

class AddSubtract : public CFreeFrameGLPlugin
{
public:
	AddSubtract();
	~AddSubtract();

	///////////////////////////////////////////////////
	// FreeFrame plugin methods
	///////////////////////////////////////////////////
    FFResult SetTextParameter(unsigned int dwIndex, const char *value) override;
    char* GetTextParameter(unsigned int index) override;

	FFResult SetFloatParameter(unsigned int dwIndex, float value) override;		
	float GetFloatParameter(unsigned int index) override;					
	FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
	FFResult InitGL(const FFGLViewportStruct *vp) override;
	FFResult DeInitGL() override;

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static FFResult __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
  {
	  *ppOutInstance = new AddSubtract();
	  if (*ppOutInstance != NULL)
		  return FF_SUCCESS;
	  return FF_FAIL;
  }


protected:	
	// Parameters
	float m_BrightnessR;
	float m_BrightnessG;
	float m_BrightnessB;
	int m_initResources;

    FFGLShader m_shader;
	GLint m_inputTextureLocation;
	GLint m_BrightnessLocation;

    double ticks;
    
    GLint ticksLoc;
    
    std::string textData;
};
