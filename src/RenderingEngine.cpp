#include "RenderingEngine.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include <vector>
#include FT_FREETYPE_H
#include "util.h"

const glm::vec3 RenderingEngine::up = glm::vec3(0.f, 1.f, 0.f);
const glm::vec3 RenderingEngine::down = glm::vec3(0.f, -1.f, 0.f);
const glm::vec3 RenderingEngine::left = glm::vec3(-1.f, 0.f, 0.f);
const glm::vec3 RenderingEngine::right = glm::vec3(1.f, 0.f, 0.f);
const glm::vec3 RenderingEngine::forward = glm::vec3(0.f, 0.f, -1.f);
const glm::vec3 RenderingEngine::backward = glm::vec3(0.f, 0.f, 1.f);
const glm::vec3 RenderingEngine::one = glm::vec3(1.f, 1.f, 1.f);
const glm::vec3 RenderingEngine::zero = glm::vec3(0.f, 0.f, 0.f);

RenderingEngine *RenderingEngine::instance = nullptr;

RenderingEngine::RenderingEngine()
        : mWindow(nullptr), cameraPos(glm::vec3(0.0f, 1.0f, 8.0f)), cameraFront(glm::vec3(0.0f, 0.0f, -3.0f)),
          cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)), cameraRight(glm::vec3()),
          projection(glm::mat4(1.f)), view(glm::mat4(1.f)),
          near_plane(0.1f), far_plane(25.f),
          deltaTime(0.0f), lastFrame(0.0f), Yaw(-90.0f), Pitch(0.0f), MouseSensitivity(0.1f), firstMouse(true),
          font_shader(0), depth_shader(0), shadow_shader(0), depth_visual_shader(0), normal_shader(0), depth_cubemap_shader(0), shadow_cubemap_shader(0),
          fontVAO(0), fontVBO(0), cubeVAO(0), cubeVBO(0), quadVAO(0), quadVBO(0), planeVAO(0), planeVBO(0),
          width(0), height(0),
          ice_texture(0), metal_texture(0),
          depthCubeMapFBO(0), depthCubeMap(0),
          depthMapFBO(0), depthMap(0), usePcf(true), usePcfKeyPress(false), useShadow(false), shadowKeyPress(false), bias(0.01),
          shadowMapWidth(1024.f), shadowMapHeight(1024.f) {
  instance = this;
  mCharMap.clear();
  points.clear();
  pointLightOriginalPos = {
          glm::vec3( 0.7f,  0.2f,  2.0f),
          glm::vec3( 2.3f, 2.f, -4.0f),
          glm::vec3(-4.0f,  2.0f, -8.0f),
          glm::vec3( 0.0f,  0.0f, -3.0f)
  };
  pointLightColors = {
          glm::vec3(1.0f, 0.6f, 0.0f),
          glm::vec3(1.0f, 0.0f, 0.0f),
          glm::vec3(1.0f, 1.0, 0.0),
          glm::vec3(0.2f, 0.2f, 1.0f)
  };
}

RenderingEngine::~RenderingEngine() {
  glDeleteVertexArrays(1, &fontVAO);
  glDeleteBuffers(1, &fontVBO);
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteVertexArrays(1, &quadVAO);
  glDeleteBuffers(1, &quadVBO);
  glDeleteVertexArrays(1, &planeVAO);
  glDeleteBuffers(1, &planeVBO);
  glDeleteFramebuffers(1, &depthCubeMapFBO);
  glDeleteTextures(1, &depthCubeMap);
  glDeleteFramebuffers(1, &depthMapFBO);
  glDeleteTextures(1, &depthMap);
  glDeleteProgram(font_shader);
  glDeleteProgram(depth_shader);
  glDeleteProgram(shadow_shader);
  glDeleteProgram(depth_visual_shader);
  glDeleteProgram(normal_shader);
  glDeleteProgram(depth_cubemap_shader);
  glDeleteProgram(shadow_cubemap_shader);
  glDeleteTextures(1, &ice_texture);
  glDeleteTextures(1, &metal_texture);

  for (auto &i : mCharMap) {
    glDeleteTextures(1, &i.second.TextureID);
  }
  mCharMap.clear();
}

bool RenderingEngine::initWindow(const std::string &title, int w, int h) {
  if (!glfwInit()) {
    std::cout << "glfw init error" << std::endl;
    return false;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  mWindow = glfwCreateWindow(w, h, title.c_str(), NULL, NULL);
  if (!mWindow) {
    std::cout << "glfw window init error" << std::endl;
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(mWindow);
  glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow *win, int w, int h) {
    glViewport(0, 0, w, h);
  });
  glfwSetCursorPosCallback(mWindow, [](GLFWwindow *w, double x, double y) {
    instance->mouseCallback(x, y);
  });
  glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwPollEvents();

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cout << "glew init error" << std::endl;
    return false;
  }

  lastX = (float) w / 2.f;
  lastY = (float) h / 2.f;
  width = w;
  height = h;

  return true;
}

void RenderingEngine::initVertex() {
  glGenVertexArrays(1, &fontVAO);
  glGenBuffers(1, &fontVBO);
  glBindVertexArray(fontVAO);
  glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glBindVertexArray(0);

  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
}

bool RenderingEngine::initFramebuffer() {
  // make depth framebuffer
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glGenFramebuffers(1, &depthMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // make depth cubemap framebuffer
  glGenTextures(1, &depthCubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glGenFramebuffers(1, &depthCubeMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return true;
}

bool RenderingEngine::initFont(const std::string &filename) {
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    std::cout << "Could not init FreeType Library" << std::endl;
    return false;
  }

  FT_Face face;
  if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
    std::cout << "Failed to load font" << std::endl;
    return false;
  }

  // Set size to load glyphs as
  FT_Set_Pixel_Sizes(face, 0, 48);

  // Disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Load first 128 characters of ASCII set
  for (GLubyte c = 0; c < 128; c++) {
    // Load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "Failed to load Glyph" << std::endl;
      continue;
    }
    // Generate texture
    GLuint font_texture;
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                 GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now store character for later use
    Character character = {
            font_texture,
            c,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
    };
//    character.printInfo();
    mCharMap.insert(std::pair<GLchar, Character>(c, character));
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  std::cout << "Total GlyphMap size: " << mCharMap.size() << std::endl;
  return true;
}

bool RenderingEngine::initShader() {
  font_shader = loadShaderFromFile("../shaders/font/font_vs.shader", "../shaders/font/font_fs.shader");
  if (!font_shader) return false;
  depth_shader = loadShaderFromFile("../shaders/shadow/depth_vs.shader", "../shaders/shadow/depth_fs.shader");
  if (!depth_shader) return false;
  shadow_shader = loadShaderFromFile("../shaders/shadow/shadow_vs.shader", "../shaders/shadow/shadow_fs.shader");
  if (!shadow_shader) return false;
  depth_visual_shader = loadShaderFromFile("../shaders/shadow/depth_visual_vs.shader", "../shaders/shadow/depth_visual_fs.shader");
  if (!depth_visual_shader) return false;
  normal_shader = loadShaderFromFile("../shaders/normal/normal_vs.shader", "../shaders/normal/normal_fs.shader");
  if (!normal_shader) return false;
  depth_cubemap_shader = loadShaderFromFile("../shaders/point_shadow/depth_vs.shader", "../shaders/point_shadow/depth_gs.shader", "../shaders/point_shadow/depth_fs.shader");
  if (!depth_cubemap_shader) return false;
  shadow_cubemap_shader = loadShaderFromFile("../shaders/point_shadow/shadow_vs.shader", "../shaders/point_shadow/shadow_fs.shader");
  if (!shadow_cubemap_shader) return false;
  return true;
}

bool RenderingEngine::initTexture() {
  ice_texture = loadTexture("../res/ice.png");
  if (!ice_texture) return false;
  metal_texture = loadTexture("../res/metal.png");
  if (!metal_texture) return false;
  return true;
}

int RenderingEngine::render() {
  while (!glfwWindowShouldClose(mWindow)) {
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(mWindow, true);

    updateDeltaTime();
    updateCamera();
    keyboardCallback();

    for (int i =0; i < pointLightOriginalPos.size(); i++) {
      points.push_back(glm::vec3(
              pointLightOriginalPos[i].x + cos(lastFrame * (0.5f + i)) * 5.f,
              pointLightOriginalPos[i].y,
              pointLightOriginalPos[i].z + sin(lastFrame * (0.5f + i)) * 5.f
      ));
    }
    renderFrame();
    renderLight();
    points.clear();
    text("near: " + std::to_string(near_plane) + ", far: " + std::to_string(far_plane),
            glm::vec2(5.f, 130.f), glm::vec3(1.f, 1.f, 0.f));
    text("camera front: [" + std::to_string(cameraFront.x) + ", " + std::to_string(cameraFront.y) + ", " +
         std::to_string(cameraFront.z) + "]", glm::vec2(5.f, 105.f), glm::vec3(1.f, 1.f, 0.f));
    text("camera pos: [" + std::to_string(cameraPos.x) + ", " + std::to_string(cameraPos.y) + ", " +
         std::to_string(cameraPos.z) + "]", glm::vec2(5.f, 80.f), glm::vec3(1.f, 1.f, 0.f));
    text("shadow bias: " + std::to_string(bias) + " (KEY: O -, P +)", glm::vec2(5.f, 55.f), glm::vec3(1.f, 1.f, 0.f));
    std::string use_pcf_str = usePcf ? "true" : "false";
    text("use pcf? " + use_pcf_str + " (KEY: TAB toggle)", glm::vec2(5.f, 30.f), glm::vec3(1.f, 1.f, 0.f));
    std::string use_shadows_str = useShadow ? "true" : "false";
    text("use shadow? " + use_shadows_str + " (KEY: SPACE toggle)", glm::vec2(5.f, 5.f), glm::vec3(1.f, 1.f, 0.f));

    glfwSwapBuffers(mWindow);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void RenderingEngine::renderScene(unsigned int shader) {
  // floor
  glm::mat4 model = glm::mat4(1.0f);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glBindVertexArray(planeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  // first cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
  model = glm::scale(model, glm::vec3(0.5f));
  glBindVertexArray(cubeVAO);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 36);
  // another cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 36);
  // another cube2
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.25));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, metal_texture);
  // cube1
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.92f, 0.f, 5.35f));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 36);
  // cube2
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-4.0f, 0.0f, 2.0));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 36);
  // cube3
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 8.0));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

void RenderingEngine::renderFrame() {
  glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), shadowMapWidth / shadowMapHeight, near_plane, far_plane);
  std::vector<glm::mat4> shadowTransforms;
  for (int i = 0; i < points.size(); i++) {
    shadowTransforms.push_back(shadowProj * glm::lookAt(points[i], points[i] + RenderingEngine::right, RenderingEngine::down));
    shadowTransforms.push_back(shadowProj * glm::lookAt(points[i], points[i] + RenderingEngine::left, RenderingEngine::down));
    shadowTransforms.push_back(shadowProj * glm::lookAt(points[i], points[i] + RenderingEngine::up, RenderingEngine::backward));
    shadowTransforms.push_back(shadowProj * glm::lookAt(points[i], points[i] + RenderingEngine::down, RenderingEngine::forward));
    shadowTransforms.push_back(shadowProj * glm::lookAt(points[i], points[i] + RenderingEngine::backward, RenderingEngine::down));
    shadowTransforms.push_back(shadowProj * glm::lookAt(points[i], points[i] + RenderingEngine::forward, RenderingEngine::down));
  }

  // 1. drawing geometry to depthcubemap
  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, shadowMapWidth, shadowMapHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glUseProgram(depth_cubemap_shader);
  for (int i = 0; i < 6; ++i) {
    glUniformMatrix4fv(glGetUniformLocation(depth_cubemap_shader, ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
  }
  glUniform1f(glGetUniformLocation(depth_cubemap_shader, "far_plane"), far_plane);
  for (int i = 0;i < points.size(); i++) {
    glUniform3fv(glGetUniformLocation(depth_cubemap_shader, ("lightPos" + std::to_string(i)).c_str()), 1, glm::value_ptr(points[i]));
  }
  renderScene(depth_cubemap_shader);

  // 2. drawing geometry
  glViewport(0, 0, width, height);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shadow_cubemap_shader);
  projection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.1f, 100.0f);
  view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  glUniformMatrix4fv(glGetUniformLocation(shadow_cubemap_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
  glUniformMatrix4fv(glGetUniformLocation(shadow_cubemap_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniform3fv(glGetUniformLocation(shadow_cubemap_shader, "viewPos"), 1, glm::value_ptr(cameraPos));
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "far_plane"), far_plane);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "bias"), bias);
  glUniform1i(glGetUniformLocation(shadow_cubemap_shader, "use_pcf"), usePcf);
  glUniform1i(glGetUniformLocation(shadow_cubemap_shader, "useShadow"), useShadow);
  // Point light 1
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].position"), points[0].x, points[0].y, points[0].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].ambient"), pointLightColors[0].x * 0.1,  pointLightColors[0].y * 0.1,  pointLightColors[0].z * 0.1);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].diffuse"), pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].specular"), pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].constant"), 1.0f);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].linear"), 0.09);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[0].quadratic"), 0.032);
  // Point light 2
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].position"), points[1].x, points[1].y, points[1].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].ambient"), pointLightColors[1].x * 0.1,  pointLightColors[1].y * 0.1,  pointLightColors[1].z * 0.1);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].diffuse"), pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].specular"), pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].constant"), 1.0f);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].linear"), 0.09);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[1].quadratic"), 0.032);
  // Point light 3
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].position"), points[2].x, points[2].y, points[2].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].ambient"), pointLightColors[2].x * 0.1,  pointLightColors[2].y * 0.1,  pointLightColors[2].z * 0.1);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].diffuse"), pointLightColors[2].x,  pointLightColors[2].y,  pointLightColors[2].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].specular") ,pointLightColors[2].x,  pointLightColors[2].y,  pointLightColors[2].z);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].constant"), 1.0f);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].linear"), 0.09);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[2].quadratic"), 0.032);
  // Point light 4
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].position"), points[3].x, points[3].y, points[3].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].ambient"), pointLightColors[3].x * 0.1,  pointLightColors[3].y * 0.1,  pointLightColors[3].z * 0.1);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].diffuse"), pointLightColors[3].x,  pointLightColors[3].y,  pointLightColors[3].z);
  glUniform3f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].specular"), pointLightColors[3].x,  pointLightColors[3].y,  pointLightColors[3].z);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].constant"), 1.0f);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].linear"), 0.09);
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "pointLights[3].quadratic"), 0.032);

  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "material.shininess"), 32.0f);
  glUniform1i(glGetUniformLocation(shadow_cubemap_shader, "material.diffuse"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ice_texture);
  for (int i = 0;i < points.size(); i++) {
    glUniform1i(glGetUniformLocation(shadow_cubemap_shader, ("depthMap[" + std::to_string(i) + "]").c_str()), 1);
    glActiveTexture(GL_TEXTURE1 + i);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
  }
  renderScene(shadow_cubemap_shader);
}

void RenderingEngine::renderLight() {
  glEnable(GL_DEPTH_TEST);
  glUseProgram(normal_shader);
  glBindVertexArray(cubeVAO);
  glUniformMatrix4fv(glGetUniformLocation(normal_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(normal_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
  glm::mat4 model = glm::mat4(1.0f);
  for (int i =0; i < points.size(); i++) {
    model = glm::translate(model, points[i]);
    model = glm::scale(model, glm::vec3(0.2f));
    glUniformMatrix4fv(glGetUniformLocation(normal_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(glGetUniformLocation(normal_shader, "LightColor"), 1, glm::value_ptr(pointLightColors[i]));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
  }
}

void RenderingEngine::text(std::string text, glm::vec2 pos, glm::vec3 color) {
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(font_shader);
  glUniform3f(glGetUniformLocation(font_shader, "textColor"), color.x, color.y, color.z);
  glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
  glUniformMatrix4fv(glGetUniformLocation(font_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(fontVAO);

  // Iterate through all characters
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++) {
    Character ch = mCharMap[*c];

    GLfloat xpos = pos.x + ch.Bearing.x * 0.5;
    GLfloat ypos = pos.y - (ch.Size.y - ch.Bearing.y) * 0.5;

    GLfloat w = ch.Size.x * 0.5;
    GLfloat h = ch.Size.y * 0.5;
    // Update VBO for each character
    GLfloat vertices[6][4] = {
            {xpos,     ypos + h, 0.0, 0.0},
            {xpos,     ypos,     0.0, 1.0},
            {xpos + w, ypos,     1.0, 1.0},

            {xpos,     ypos + h, 0.0, 0.0},
            {xpos + w, ypos,     1.0, 1.0},
            {xpos + w, ypos + h, 1.0, 0.0}
    };
    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices),
                    vertices); // Be sure to use glBufferSubData and not glBufferData

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    pos.x += (ch.Advance >> 6) *
             0.5; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
}

void RenderingEngine::mouseCallback(double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  xoffset *= MouseSensitivity;
  yoffset *= MouseSensitivity;

  Yaw += xoffset;
  Pitch += yoffset;

  if (Pitch > 89.0f)
    Pitch = 89.0f;
  if (Pitch < -89.0f)
    Pitch = -89.0f;
}

void RenderingEngine::keyboardCallback() {
  float speed = 2.5f;
  if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    speed = 7.5f;
  float velocity = speed * deltaTime;

  if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraFront * velocity;
  if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraFront * velocity;
  if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= cameraRight * velocity;
  if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += cameraRight * velocity;
  if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
    cameraPos += cameraUp * velocity;
  if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
    cameraPos -= cameraUp * velocity;
  if (glfwGetKey(mWindow, GLFW_KEY_P) == GLFW_PRESS)
    bias += 0.01;
  if (glfwGetKey(mWindow, GLFW_KEY_O) == GLFW_PRESS)
    bias -= 0.01;

  if (glfwGetKey(mWindow, GLFW_KEY_TAB) == GLFW_PRESS && !usePcfKeyPress) {
    usePcf = !usePcf;
    usePcfKeyPress = true;
  }
  if (glfwGetKey(mWindow, GLFW_KEY_TAB) == GLFW_RELEASE) {
    usePcfKeyPress = false;
  }

  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowKeyPress) {
    useShadow = !useShadow;
    shadowKeyPress = true;
  }
  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_RELEASE) {
    shadowKeyPress = false;
  }
}

void RenderingEngine::updateDeltaTime() {
  auto currentFrame = (float) glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
}

void RenderingEngine::updateCamera() {
  glm::vec3 f;
  f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  f.y = sin(glm::radians(Pitch));
  f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  cameraFront = glm::normalize(f);
  cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.f, 1.f, 0.f)));
  cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}