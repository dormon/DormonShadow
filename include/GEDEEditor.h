#pragma once

#include<Draw2D.h>

namespace gde{
  class Function{
    public:
      std::shared_ptr<Draw2D>draw2D;
      std::string functionName;
      std::vector<std::string>inputNames;
      std::string outputName;
      glm::vec4 backgroundColor = glm::vec4(0,0,0,.8);
      glm::vec4 captionBackgrounColor = glm::vec4(0.0,0.1,0.0,1);
      glm::vec4 captionColor = glm::vec4(0,1,0,1);
      glm::vec4 lineColor = glm::vec4(0,1,0,1);
      glm::vec4 textColor = glm::vec4(0,1,0,1);
      size_t captionFontSize = 10;
      size_t fontSize = 8;
      size_t margin = 2;
      size_t captionMargin = 2;
      size_t textIndent = 2;
      size_t inputSpacing = 2;
      size_t inputOutputDistance = 10;
      size_t inputRadius = 2;
      size_t outputRadius = 2;
      size_t lineWidth = 1;

      size_t captionText;
      std::vector<size_t>inputText;
      std::vector<size_t>inputCircle;
      size_t outputText;
      size_t outputCircle;
      size_t rectangleLines[4];
      size_t captionLine;
      size_t rectangleTriangles[2];
      size_t captionTriangles[2];
      size_t node;
      Function(
          std::shared_ptr<Draw2D> const&draw2D,
          std::string             const&fce,
          std::vector<std::string>const&inputs,
          std::string             const&output){
        this->draw2D = draw2D;
        this->functionName = fce;
        this->inputNames = inputs;
        this->outputName = output;
      }
      void create();
      ~Function(){
        draw2D->deleteNode(this->node);
      }
  };
}
