#include <common/camera.hpp>

Camera::Camera(const glm::vec3 Eye, const glm::vec3 Target)
{
    eye = Eye;
    target = Target;
}

void Camera::calculateMatrices()
{
    // Calculate camera vectors
    calculateCameraVectors();

    // Calculate the view matrix
    //view = orientation.matrix() * Maths::translate(-eye);
    view = Maths::lookAt(eye, eye + front, worldUp);

    // Calculate the projection matrix
    projection = Camera::perspective(fov, aspect, near, far);
}

void Camera::calculateCameraVectors()
{
    front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
    right = Maths::normalise(Maths::cross(front, worldUp));
    up = Maths::cross(right, front);
}

void Camera::quaternionCamera()
{
    // Calculate camera orientation quaternion from the Euler angles
    Quaternion newOrientation(-pitch, yaw);

    // Apply SLERP
    orientation = Maths::SLERP(orientation, newOrientation, 0.2f);

    // Calculate the view matrix
    view = orientation.matrix() * Maths::translate(-eye);

    // Calculate the projection matrix
    projection = Camera::perspective(fov, aspect, near, far);

    // Calculate camera vectors from view matrix
    right = glm::vec3(view[0][0], view[1][0], view[2][0]);
    up = glm::vec3(view[0][1], view[1][1], view[2][1]);
    front = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}

glm::mat4 Camera::perspective(float fov, float aspect, float near, float far)
{
    float top = near * (tan(fov / 2));
    float bottom = -top;
    float right = aspect * top;
    float left = -right;

    glm::mat4 projectionMatrix = {};
    projectionMatrix[0][0] = near / right;
    projectionMatrix[1][1] = near / top;
    projectionMatrix[2][2] = -(far + near) / (far - near);
    projectionMatrix[2][3] = -1.0f;
    projectionMatrix[3][2] = -(2 * far * near) / (far - near);

    return projectionMatrix;
}

void Camera::thirdPersonCamera() {
    glm::vec3 offset = glm::vec3(0.0f, -1.0f, -3.0f);

    // Calculate camera orientation quaternion from the Euler angles
    Quaternion newOrientation(-pitch, yaw);

    // Apply SLERP
    orientation = Maths::SLERP(orientation, newOrientation, 0.2f);

    // Calculate the view matrix
    view = orientation.matrix() * Maths::translate(-eye + offset);

    // Calculate the projection matrix
    projection = Camera::perspective(fov, aspect, near, far);

    // Calculate camera vectors from view matrix
    right = glm::vec3(view[0][0], view[1][0], view[2][0]);
    up = glm::vec3(view[0][1], view[1][1], view[2][1]);
    front = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}

//NO GLM LEFT

