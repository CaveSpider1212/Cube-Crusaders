## Over-Terrain Vehicle (Cube Crusaders)

<img width="359" height="257" alt="image" src="https://github.com/user-attachments/assets/61bfa2d5-5928-404c-858b-fa656ba5d29f" />

#### Description

This was a semester-long project for ENES 100 at the University of Maryland in which we had to construct an over-terrain vehicle (OTV) to try and complete 3 navigation objectives and 3 mission objectives. Each class section was split into groups, where each group was given a distinct mission. Our group's mission was about material identification.

The 3 navigation objectives were:
- Navigate to within 150 mm of the mission site
- Navigate past the three obstacles
- Navigate into the destination zone (either over the log or under the limbo)

The 3 mission objectives for our mission were:
- Lift the cube off the ground
- Identify the material of the cube (either foam or plastic)
- Identify the weight class of the cube (either light, medium, or heavy weight)

#### Development

Our group, which consisted of 8 students, split up into multiple sub-groups, with each sub-group working on a separate task (chassis, CAD, mechanical, mission, navigation, etc.) for the OTV. Personally, I was part of the group in charge of programming the navigation and mission. We approached this project mainly by working separately at first, then integrating all of the sub-systems together.

The chassis is made out of wood and was produced by laser cutting. The parts on the OTV were designed using OnShape's CAD software and 3D printed using Prusa. The OTV was programmed using Arduino.

Our approach for the navigation algorithm was the following:
- For turning, turn a certain number of degrees (calculated by subtracting the desired orientation angle from the OTV's current orientation angle) as close as it can, then "nudge" the OTV to the desired orientation using the overhead vision system
- When navigating through the obstacles, have the OTV go forward until it detects an obstacle ahead of it; once detected, have the OTV move to the next zone above/below it (depending on where it started) and repeat

<img width="525" height="261" alt="image" src="https://github.com/user-attachments/assets/0505f6c3-f4ac-47a3-962a-50cd763df401" />

- When navigating to the destination zone, move the OTV up/down to the same vertical position as the limbo and have the OTV move under

The approach we had for the mission objective was the following:
- Drive the cube into the arena wall to help pick it up off the ground
- Push the compressor (which has a force sensor attached to it) onto the cube to determine the material (if the sensor reads above a certain threshold, it's plastic; otherwise, it's foam)

We weren't able to figure out a systematic approach for determining the weight of the cube in time, so we didn't do much for that objective.

#### Results

While our mission objectives were very inconsistent during testing (but did fairly well during our scored runs), our navigation was consistently doing well. As a result, we were the best-performing group out of all of the material identification teams in all 21 ENES 100 class sections.

#### Videos

https://drive.google.com/file/d/1VvsLtdQQrPcpOaQhiBHc29GOkdY9uJwi/view?usp=sharing
