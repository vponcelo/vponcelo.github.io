function [depthValue,x,y] = worldToPixel(vector3D)
% Counter step to depthToWorld function applying algebraic transformations
% http://graphics.stanford.edu/~mdfisher/Kinect.html
% This function ignore the distortion effects of the lens, but still give
% reasonably accurate results.

FX_D = 1.0 / 5.9421434211923247e+02;
FY_D = 1.0 / 5.9104053696870778e+02;
CX_D = 3.3930780975300314e+02;
CY_D = 2.4273913761751615e+02;

depthValue = MetersToRaw(vector3D.z);
x = vector3D.x / (1000 * vector3D.z * FX_D) + CX_D;
y = vector3D.y / (1000 * vector3D.z * FY_D) + CY_D;

end

function depthValue = MetersToRaw(depthMeters)

    depthValue = 1.0 / (depthMeters * -0.0030711016) + 3.3309495161 / 0.0030711016;
    
    if (depthValue < 2047)
        depthValue = round(depthValue);
    else
        depthValue = -1;
    end
end

