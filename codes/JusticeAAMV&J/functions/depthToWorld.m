function vector3D = depthToWorld(x,y,depthValue)

% http://graphics.stanford.edu/~mdfisher/Kinect.html
% This function ignore the distortion effects of the lens, but still give
% reasonably accurate results.

FX_D = 1.0 / 5.9421434211923247e+02;
FY_D = 1.0 / 5.9104053696870778e+02;
CX_D = 3.3930780975300314e+02;
CY_D = 2.4273913761751615e+02;

depth = RawDepthToMeters(depthValue);
vector3D.x = (x - CX_D) * depth * FX_D * 1000;
vector3D.y = (y - CY_D) * depth * FY_D * 1000;
vector3D.z = depth;

end

function meters = RawDepthToMeters(depthValue)

    if (depthValue < 2047)
        meters = 1.0 / (double(depthValue) * -0.0030711016 + 3.3309495161); 
    else
        meters = 0;
    end
    
end

