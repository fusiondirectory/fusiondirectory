<!-- 
	Div container with PHP module checks 
-->
<div style='float:left;width:50%;'> 

	<!-- Set content styles -->	
	<div class='default' style='margin:10px;'>
		<!-- Beginning PHP module check output -->
		<h2 class='step2_container_title'>{t}PHP module and extension checks{/t}</h2>
		<!-- {cycle reset=true values="#F0F0F0, #FFF"} -->
		{foreach from=$basic item=val key=key}
			<!-- Entry {$key} -->		
			{if $basic[$key].SOLUTION != "" && !$basic[$key].RESULT}
		
				<!-- Add ability to display info popup -->	
				<div class='step2_entry_container_info' style='background-color:{cycle values="#F0F0F0, #FFF"}'>
			{else}

				<!-- Normal entry everything is fine -->
				<div class='step2_entry_container' style='background-color:{cycle values="#F0F0F0, #FFF"}'>	
			{/if}
			
			<div class='step2_entry_name'>{$basic[$key].NAME}</div>
			<div class='step2_entry_status'>

			{if $basic[$key].RESULT}
				<div class='step2_successful'>{t}Ok{/t}</div>
			{else}
				{if $basic[$key].MUST}
					<div class='step2_failed'>{t}Error{/t}</div>
				{else}
					<div class='step2_warning'>{t}Warning{/t}</div>
				{/if}
				{if $basic[$key].SOLUTION != ""}
	
				<!-- Inforamtion popup -->
				<div class='solution_visible' id='sol_{$key}'>
					{if $basic[$key].MUST}
						<div class='step2_failed_text' style="background-image: url('images/small_error.png')">
						{$basic[$key].SOLUTION}
						<b>{t}GOsa will NOT run without fixing this.{/t}</b>
					{else}
						<div class="step2_warning_text" style="background-image: url('images/small_warning.png')">
						{$basic[$key].SOLUTION}
						<b>{t}GOsa will run without fixing this.{/t}</b>
					{/if}
					</div>
				</div>
				{/if}
			{/if}
				</div>
			</div>
		{/foreach}
	</div>
</div>


<!-- 
	Div container with PHP config checks 
-->
<div style='width:50%; float:left;'>

	<!-- Set content styles -->	
	<div class='default' style='margin:10px;'>

		<!-- Beginning PHP config check output -->
		<h2 class='step2_container_title'>{t}PHP setup configuration{/t} (<a style='text-decoration:underline' href='?info' target='_blank'>{t}show information{/t})</a></h2>

		<!-- {cycle reset=true values="#F0F0F0, #FFF"} -->
		{foreach from=$config item=val key=key}

			<!-- Entry {$key} -->
            {if $config[$key].SOLUTION != "" && !$config[$key].RESULT}

                <!-- Add ability to display info popup -->
                <div class='step2_entry_container_info' style='background-color:{cycle values="#FFF, #F0F0F0"}'>
            {else}

                <!-- Normal entry everything is fine -->
                <div class='step2_entry_container' style='background-color:{cycle values="#FFF, #F0F0F0"}'>
            {/if}


			<div class='step2_entry_name'>{$config[$key].NAME}</div>
			<div class='step2_entry_status'>
			{if $config[$key].RESULT}
				<div class='step2_successful'>{t}Ok{/t}</div>
			{else}

				{if $config[$key].MUST}
					<div class='step2_failed'>{t}Error{/t}</div>
				{else}
					<div class='step2_warning'>{t}Warning{/t}</div>
				{/if}

				{if $config[$key].SOLUTION != ""}
	
				<!-- Inforamtion popup -->
				<div class='solution_visible' id='sol_config_{$key}'>
					{if $config[$key].MUST}
						<div class='step2_failed_text' style="background-image: url('images/small_error.png')">
						{$config[$key].SOLUTION}
						<b>{t}GOsa will NOT run without fixing this.{/t}</b>
					{else}
						<div class='step2_warning_text' style="background-image: url('images/small_warning.png')">
						{$config[$key].SOLUTION}
						<b>{t}GOsa will run without fixing this.{/t}</b>
					{/if}
					</div>
				</div>
				{/if}
			{/if}
			</div>
			</div>
		{/foreach}


	</div>
</div>

<div style="clear:both"></div>

