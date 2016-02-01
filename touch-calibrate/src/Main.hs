module Main where

{-
import System.Environment
import Graphics.X11.Xlib.Display
import Graphics.X11.Xlib.Event
import Graphics.X11.Xlib.Misc
import Graphics.X11.Xlib.Screen
import Graphics.X11.Xlib.Types
import Graphics.X11.Xlib.Window
import Graphics.X11.Types
import Foreign.Ptr

main :: IO ()
main = allocaXEvent $ \event -> allocaSetWindowAttributes $ \attributes -> exec attributes event

exec :: Ptr SetWindowAttributes -> Ptr XEvent -> IO ()
exec attributes event = do
  displayName <- getEnv "DISPLAY"
  display <- openDisplay displayName
  let screen = screenOfDisplay display 0
  let sx = widthOfScreen screen
  let sy = heightOfScreen screen
  window <- createWindow display
                         (rootWindowOfScreen screen)
                         0 0
                         sx sy
                         0
                         (defaultDepthOfScreen screen)
                         inputOutput
                         (defaultVisualOfScreen screen)
                         0
                         nullPtr
  mapWindow display window
  maskEvent display buttonPressMask event
  putStrLn displayName
-}

import Graphics.UI.GLUT
--import Graphics.UI.GLUT.Window
--import Graphics.UI.GLUT.Initialization
--import Graphics.UI.GLUT.Callbacks.Window
--import Graphics.UI.GLUT.Begin

import Graphics.Rendering.OpenGL


col :: GLfloat -> GLfloat -> GLfloat -> IO ()
col r g b = color $ Color3 r g b

translate2 :: GLfloat -> GLfloat -> IO ()
translate2 x y = translate $ Vector3 x y 0

scale2 :: GLfloat -> GLfloat -> IO ()
scale2 x y = scale x y 0

zoom2 :: (GLfloat, GLfloat) -> (GLfloat, GLfloat) -> IO ()
zoom2 (x0, y0) (x1, y1) = do
  translate2 x0 y0
  scale2 (x1 - x0) (y1 - y0)

saveMatrix :: IO () -> IO ()
saveMatrix action = do
  let mat = matrix Nothing :: StateVar (GLmatrix GLfloat)
  v <- get mat
  action
  mat $= v

rectangle :: IO ()
rectangle = renderPrimitive Quads $
            mapM_ (\(x, y) -> vertex $ Vertex3 x y 0) points
  where
    points :: [(GLfloat, GLfloat)]
    points  = [ (0, 0)
              , (0, 1)
              , (1, 1)
              , (1, 0)
              ]

rectangleAt :: (GLfloat, GLfloat) -> (GLfloat, GLfloat) -> IO ()
rectangleAt p0 p1 = saveMatrix $ do
  zoom2 p0 p1
  rectangle

main :: IO ()
main = do
  getArgsAndInitialize
  window <- createWindow "Calibrate"
  fullScreen
  displayCallback $= display
  mouseCallback $= Just mouse
  mainLoop
  reportErrors
  getLine >>= putStrLn

display :: DisplayCallback
display = do
  clear [ ColorBuffer ]
  matrixMode $= Modelview 0
  loadIdentity
  translate2 (-1) (-1)
  scale2 2 2
  col 0 0 0
  rectangle
  col 1 1 1
  let d = 0.2
  rectangleAt (0, d) (d, 1 - d)
  rectangleAt (d, 0) (1 - d, d)
  rectangleAt (1 - d, d) (1, 1 - d)
  rectangleAt (d, 1 - d) (1 - d, 1)
  col 1 0 0
  rectangleAt (0.4, 0.4) (0.6, 0.6)
  flush

mouse :: MouseButton -> KeyState -> Position -> IO ()
mouse button state pos = do
  putStrLn $ show button ++ ", " ++ show state ++ ", " ++ show pos
