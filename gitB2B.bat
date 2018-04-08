git init
git remote add origin https://iz4tow:Fiwoldiois01@github.com/iz4tow/SERRA-AUTOMATICA-GMAIL.git
git add *
set /p commento=INSERIRE COMMENTO PRIMA DELLA COMMIT: 
git commit -m "%date% - %commento%"
git push origin master
pause > nul