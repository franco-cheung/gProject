--
-- CS488 -- Introduction to Computer Graphics
-- 
-- a3mark.lua
--
-- A very simple scene creating a trivial puppet.  The TAs will be
-- using this scene as part of marking your assignment.  You'll
-- probably want to make sure you get reasonable results with it!

rootnode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
red2 = gr.material({0.9, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
blue2 = gr.material({0.0, 0.0, 0.9}, {0.1, 0.1, 0.1}, 10)
blue3 = gr.material({0.0, 0.0, 0.8}, {0.1, 0.1, 0.1}, 10)
blue4 = gr.material({0.0, 0.0, 0.7}, {0.1, 0.1, 0.1}, 10)
blue5 = gr.material({0.0, 0.0, 0.6}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
green2 = gr.material({0.0, 0.9, 0.0}, {0.1, 0.1, 0.1}, 10)
green3 = gr.material({0.0, 0.8, 0.0}, {0.1, 0.1, 0.1}, 10)
green4 = gr.material({0.0, 0.7, 0.0}, {0.1, 0.1, 0.1}, 10)
green5 = gr.material({0.0, 0.6, 0.0}, {0.1, 0.1, 0.1}, 10)
green6 = gr.material({0.0, 0.5, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 10)
white2 = gr.material({0.9, 0.9, 0.9}, {0.1, 0.1, 0.1}, 10)
white3 = gr.material({0.8, 0.8, 0.8}, {0.1, 0.1, 0.1}, 10)
white4 = gr.material({0.7, 0.7, 0.7}, {0.1, 0.1, 0.1}, 10)
white5 = gr.material({0.6, 0.6, 0.6}, {0.1, 0.1, 0.1}, 10)

yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)

tankBody = gr.joint('body Joint', {1.0, 1.0, 1.0}, {1.0, 1.0, 1.0})
rootnode:add_child(tankBody)

body = gr.sphere('body')
tankBody:add_child(body)
--tankBody:scale(0.5, 0.5, 0.5);
--tankBody:translate(25, 10, 10)
body:set_material(white)

-- shoulderJoint = gr.joint('shoulder Joint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
-- tankBody:add_child(shoulderJoint)

-- shoulder = gr.sphere('shoulder')
-- shoulderJoint:add_child(shoulder)
-- shoulder:translate(10, 25, 25)
-- shoulder:set_material(blue)
-- headJoint = gr.joint('shoulder Joint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
-- tankBody:add_child(headJoint)



return rootnode
