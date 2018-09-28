use Pokemon;

#1
select name
from Trainer, CatchedPokemon
where Trainer.id = owner_id
group by Trainer.id
having count(*)>=3
order by count(*) desc;


#2
select name 
from
(select *, count(*) as cnt 
	from Pokemon P 
	group by P.type 
	having count(*) 
	order by count(*) desc, P.type 
	limit 2) as T
order by name;


#3
select name
from Pokemon
where name like '_o%'
order by name;


#4
select distinct C.nickname 
from CatchedPokemon C
where C.level>=50 
order by C.nickname;


#5
select name
from Pokemon
where name like '______'
order by name;


#6
select name 
from Trainer 
where hometown = 'Blue City' 
order by name;


#7
select distinct hometown 
from Trainer;


#8
select avg(level) 
from CatchedPokemon C, Trainer T 
where C.owner_id = T.id and T.name = 'Red';


#9
select C.nickname 
from CatchedPokemon C 
where C.nickname not like 'T%' 
order by C.nickname;


#10
select C.nickname
from CatchedPokemon C
where C.level>=50 and C.owner_id>=6
order by C.nickname;


#11
select id, name
from Pokemon
order by id;


#12
select nickname 
from CatchedPokemon 
where level<=50 
order by level;


#13
select P.name, P.id
from Pokemon P, CatchedPokemon C, Trainer T
where T.hometown = 'Sangnok City' and T.id = C.owner_id and C.pid = P.id
order by P.id;


#14
select C.nickname 
from CatchedPokemon C, Trainer T, Gym G , Pokemon P 
where G.leader_id = T.id and T.id = C.owner_id and C.pid = P.id and P.type = 'Water' 
order by C.nickname;


#15
select count(*) 
from Pokemon P, Evolution E 
where P.id = E.before_id;


#16
select count(*) 
from Pokemon 
where type = 'Water' or type = 'Electric' or type = 'Psychic';


#17
select count(distinct C.pid) 
from CatchedPokemon C, Trainer T 
where T.hometown = 'Sangnok City' and C.owner_id = T.id;


#18
select max(C.level) 
from CatchedPokemon C, Trainer T 
where T.hometown = 'Sangnok City' and C.owner_id = T.id;


#19
select count(distinct P.type) 
from CatchedPokemon C, Gym G, Pokemon P 
where C.owner_id = G.leader_id and G.city = 'Sangnok City' and C.pid = P.id;


#20
select T.name, count(*) as cnt_catched_pokemon 
from CatchedPokemon C, Trainer T 
where C.owner_id = T.id and T.hometown = 'Sangnok City' 
group by T.id 
order by count(*);


#21
select name
from Pokemon
where name like 'A%' or name like 'E%' or name like 'I%' or name like 'U%';


#22
select type, count(*) as count_type
from Pokemon
group by type
order by count(*), type;


#23
select distinct T.name 
from Trainer T, CatchedPokemon C 
where C.level<=10 and C.owner_id = T.id 
order by T.name;


#24
select T.hometown, avg(C.level) as hometown_avg_level 
from Trainer T, CatchedPokemon C 
where T.id = C.owner_id 
group by T.hometown 
order by avg(C.level);


#25
select distinct ST.name 
from 
(select P.name 
	from Pokemon P, CatchedPokemon C, Trainer T 
	where P.id = C.pid and C.owner_id = T.id and T.hometown = 'Sangnok City') as ST , 
(select P.name 
	from Pokemon P, CatchedPokemon C, Trainer T 
	where P.id = C.pid and C.owner_id = T.id and T.hometown = 'Brown City') as BT 
where ST.name = BT.name 
order by ST.name;


#26
select nickname
from CatchedPokemon
where nickname like '% %'
order by nickname desc;


#27
select T.name, max(C.level) 
from CatchedPokemon C, Trainer T 
where C.owner_id = T.id 
group by T.id 
having count(*)>=4 
order by T.name;


#28
select T.name, avg(C.level) as avg_catched 
from Trainer T, CatchedPokemon C, Pokemon P 
where T.id = C.owner_id and C.pid = P.id and (P.type = 'Normal' or P.type = 'Electric') 
group by T.name 
having avg(C.level) 
order by avg(C.level);


#29
select P.name as Pokemon_name, T.name as Trainer_name, C.description as City_description 
from Pokemon P, CatchedPokemon CP, Trainer T, City C 
where P.id = 152 and CP.pid = P.id and CP.owner_id = T.id and T.hometown = C.name 
order by CP.level desc;


#30
select E1.before_id as id, P1.name as 1_name, P2.name as 2_name, P3.name as 3_name
from Pokemon P1, Pokemon P2, Pokemon P3, Evolution E1, Evolution E2
where P1.id = E1.before_id and P2.id = E1.after_id and E1.after_id = E2.before_id and P3.id = E2.after_id
order by E1.before_id;


#31
select name 
from Pokemon 
where id>=10 
order by name;


#32
select P.name
from Pokemon P 
where P.id not in
(select pid from CatchedPokemon)
order by P.name;


#33
select sum(C.level) 
from Trainer T, CatchedPokemon C 
where T.name = 'Matis' and T.id = C.owner_id;


#34
select T.name
from Trainer T, Gym G
where T.id = G.leader_id
order by T.name;


#35
select P1.type, max((P1.cnt / P2.total)*100) as ratio 
from 
(select *, count(*) as cnt 
	from Pokemon P 
	group by P.type) as P1, 
(select count(*) as total 
	from Pokemon) as P2 
order by ratio;


#36
#중복처리 안함.
select T.name 
from Trainer T, CatchedPokemon C 
where T.id = C.owner_id and C.nickname like 'A%' 
order by T.name;


#37
select C.name, max(C.s) as sum 
from 
(select T.name, sum(C.level) as s 
	from CatchedPokemon C, Trainer T 
	where C.owner_id = T.id 
	group by T.id) as C;


#38
select P2.name as name 
from Pokemon P1, Pokemon P2, Pokemon P3, Evolution E1, Evolution E2 
where P1.id = E1.before_id and P2.id = E1.after_id and E1.after_id = E2.before_id and P3.id = E2.after_id
union
select P2.name as name 
from Pokemon P1, Pokemon P2, Evolution E 
where P1.id = E.before_id and P2.id = E.after_id 
and P2.id 
not in (select before_id from Evolution) 
and P1.id 
not in (select after_id from Evolution)
order by name;


#39
select distinct T.name 
from Trainer T, CatchedPokemon C1, CatchedPokemon C2 
where T.id = C1.owner_id and T.id = C2.owner_id and C1.pid = C2.pid and C1.id<>C2.id 
order by T.name;


#40
select T1.name, T1.nickname 
from 
(select C.name, CP.level, CP.nickname 
	from Trainer T, City C, CatchedPokemon CP 
	where T.hometown = C.name and CP.owner_id = T.id) as T1, 
(select C.name, max(CP.level) as level 
	from Trainer T, City C, CatchedPokemon CP 
	where T.hometown = C.name and CP.owner_id = T.id 
	group by C.name) as T2 
where T1.name = T2.name and T1.level = T2.level 
order by T1.name;
