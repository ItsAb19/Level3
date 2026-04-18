/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

API (DLL) specific definitions
----------------------------------------------------------------------------------
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   Jarek Francik
   jarek@kingston.ac.uk
*********************************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the MY3DGL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// MY3DGL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MY3DGL_EXPORTS
#define MY3DGL_API __declspec(dllexport)
#else
#define MY3DGL_API __declspec(dllimport)
#endif

// allows for exporting selected glm structures
#ifdef GLM_SETUP_INCLUDED
template struct MY3DGL_API glm::vec<3, float, glm::highp>;
#endif

// allows for exporting std::string (don't ask me how!)
struct MY3DGL_API std::_Container_base12;
union MY3DGL_API std::_String_val<std::_Simple_types<char>>::_Bxty;
template class MY3DGL_API std::_String_val<std::_Simple_types<char>>;
template class MY3DGL_API std::_Compressed_pair<std::allocator<char>, std::_String_val<std::_Simple_types<char>>, true>;
template class MY3DGL_API std::basic_string<char, std::char_traits<char>, std::allocator<char>>;