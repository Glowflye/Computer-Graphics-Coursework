#include <common/camera.hpp>

Camera::Camera(const glm::vec3 Eye, const glm::vec3 Target)
{
    eye = Eye;
    target = Target;
}

void Camera::calculateMatrices()
{
    //// Calculate the view matrix
    //view = glm::lookAt(eye, target, worldUp);

    // Calculate camera vectors - ADDED
    calculateCameraVectors();

    // Calculate the view matrix - ADDED
    view = glm::lookAt(eye, eye + front, worldUp);

    // Calculate the projection matrix
    projection = glm::perspective(fov, aspect, near, far);
}

void Camera::calculateCameraVectors()
{
    front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
    right = glm::normalize(Maths::cross(front, worldUp));
    up = Maths::cross(right, front);
}

void Camera::quaternionCamera()
{
    // Calculate camera orientation quaternion from the Euler angles
    /*Quaternion orientation(-pitch, yaw);*/
    // Calculate camera orientation quaternion from the Euler angles
    Quaternion newOrientation(-pitch, yaw);

    // Apply SLERP
    orientation = Maths::SLERP(orientation, newOrientation, 0.2f);

    // Calculate the view matrix
    view = orientation.matrix() * Maths::translate(-eye);

    // Calculate the projection matrix
    projection = glm::perspective(fov, aspect, near, far);

    // Calculate camera vectors from view matrix
    right = glm::vec3(view[0][0], view[1][0], view[2][0]);
    up = glm::vec3(view[0][1], view[1][1], view[2][1]);
    front = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}
