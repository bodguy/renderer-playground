#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include "../util.h"
#include "Transform.h"

class Camera {
  public:
    Camera();
    explicit Camera(const glm::vec3& pos);
    ~Camera();

    bool Init();
    Transform* GetTransform();
    float GetFieldOfView() const;
    glm::vec4 GetBackgroundColor() const;
    float GetNearClipPlane() const;
    float GetFarClipPlane() const;
    float GetAspectRatio() const;
    float GetHdrExposure() const;
    bool IsHdr() const;
    bool IsOrthographic() const;
    Rect<unsigned int> GetPixelRect() const;
    glm::mat4 GetWorldToCameraMatrix();
    glm::mat4 GetCameraToWorldMatrix();
    glm::mat4 GetProjectionMatrix();
    unsigned int GetHDRFBO() const;

    void Render();

    void SetPixelRect(const Rect<unsigned int>& r);
    void SetHdr(bool f);
    void SetOrthographic(bool f);
    void SetFieldOfView(float degree);
    void SetBackgroundColor(glm::vec4 color);
    void SetNearClipPlane(float near);
    void SetFarClipPlane(float far);
    void SetAspectRatio(int w, int h);
    void SetHdrExposure(float e);

  private:
    Transform transform;
    Rect<unsigned int> pixelRect;
    Rect<float> normalizedRect;
    bool hdr;
    bool orthographic;
    float fieldOfView;
    glm::vec4 backgroundColor;
    float nearClipPlane, farClipPlane;
    float aspectRatio;
    float exposure;
    unsigned int targetTexture;  // @TODO
    bool useOcclusionCulling;    // @TODO
    glm::mat4 worldToCameraMatrix, cameraToWorldMatrix, projectionMatrix;
    unsigned int quadVAO, quadVBO, hdrFBO, hdrColorTexture, hdrRboDepth, hdrShader;
};

#endif  // CAMERA_H
