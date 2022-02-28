#include "ModelRenderer.h"

int main()
{
	ModelRenderer* myApp = new ModelRenderer();
	myApp->Run("Model Renderer", 1920, 1080, false);
	delete myApp;
	return 0;
}